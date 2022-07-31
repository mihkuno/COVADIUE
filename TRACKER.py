import json
import os.path
import urllib.request
from datetime import date
from selenium.webdriver.common.by import By
from multiprocessing import Process, Manager, Lock
from selenium.webdriver import Chrome, ChromeOptions
from selenium.webdriver.chrome.service import Service
from webdriver_manager.chrome import ChromeDriverManager
from selenium.webdriver.common.desired_capabilities import DesiredCapabilities


web = [
    'https://www.worldometers.info/coronavirus/#main_table',
    'https://www.worldometers.info/coronavirus/country/philippines/',
    'https://www.worldometers.info/coronavirus/weekly-trends/#weekly_table',
]

url = "http://192.168.1.12/covscrape?"                     

# https://doh.gov.ph/covid19tracker
# bed occupancy 
# number of facilities
# region/province scope
# equipment availability 
# occupied and vacancies

def webdriver(mode):
    # chromium driver library
    settings = ChromeOptions()
    settings.add_argument('headless')

    caps = DesiredCapabilities().CHROME
    caps["pageLoadStrategy"] = mode # "normal"  #  complete
    # caps["pageLoadStrategy"] = "eager"        #  interactive
    #caps["pageLoadStrategy"] = "none"

    driver = ChromeDriverManager().install()
    driver = Chrome(
        service=Service(driver),
        desired_capabilities=caps)
    
    return driver


def scrape1(p_snap, p_ktask, p_lock, data_snapshot):    
    
    print(data_snapshot)
    print(web[1])
    driver = webdriver("eager")
    driver.get(web[1])

    # extract data
    tcases = driver.find_element(By.XPATH, "//div[@class='content-inner']//div[4]").text[19:]
    tdeath = driver.find_element(By.XPATH, "//div[@class='content-inner']//div[5]").text[8:]
    trecov = driver.find_element(By.XPATH, "//div[@class='content-inner']//div[6]").text[11:]
    mupdat = driver.find_element(By.XPATH, "//div[@id='news_block']").text.splitlines()    
    
    if mupdat and tcases and tdeath and trecov:
        driver.quit()
    
    # updates
    ccdate = mupdat[1][:-6].replace(" ", ".")
    ncases = mupdat[3][:mupdat[3].index('new cases')-1] if 'new cases' in mupdat[3] else "0"
    
    if 'new deaths' in mupdat[3]:
        start = mupdat[3].index('and')+4
        end = mupdat[3].index('new deaths')-1
        ndeath = mupdat[3][start:end]
        print(start, end)
    else:
        ndeath = "0"
        
    print(mupdat)
    print(ndeath)
    print(ccdate)     
        

    # check date of snapshot    
    if data_snapshot and ccdate == data_snapshot['nwdat']:
        print('hello there!!!!!!------------------------')
        p_ktask.value = True
        return 0   
    else:
        print('NIIILIIKAAYYY________________________________')
        
        with p_lock:
            p_snap.update({
                "cases":f"{tcases}",
                "death":f"{tdeath}",
                "recov":f"{trecov}",
                "nwdat":f"{ccdate}",
                "nwcas":f"{ncases}",
                "nwdea":f"{ndeath}"
            })
        
        global url
        
        for h,v in p_snap.items():
            url += f"{h}={v}&"
        url = url[:-1]
        
        print(url)
        urllib.request.urlopen(url)   
        
  
def scrape2(p_snap, p_ktask, p_lock):

    td_column = 4 # column 
    
    print(web[2])
    driver = webdriver('eager')
    driver.get(web[2])
    
    if p_ktask.value:
        print('bef quit 222')
        driver.quit()
        return 0
    else:    
        button = "//nav[@id='ctabstoday']//ul[@class='nav nav-tabs']//li[@id='nav-asia-tab']"
        tab_asia = driver.find_element(By.XPATH, button)
        tab_asia.click()
        
        rows = "//tbody/tr"
        path = driver.find_elements(By.XPATH, rows)
        
        for row in path:
            if p_ktask.value:
                print('row quit')
                driver.quit()
                return 0
            col = row.find_elements(By.TAG_NAME, "td")
            bucket = [x.text for x in col]
            if bucket and bucket[1] in "Philippines":
                driver.quit()
                column = bucket[td_column]
                wkperc = column.replace("+","%2B") if "+" in column else wkperc.replace("-",'%2D')    
            
                with p_lock:
                    p_snap.update({"wkper":f"{wkperc}"})
                
                global url
    
                for h,v in p_snap.items():
                    url += f"{h}={v}&"
                url = url[:-1]
                
                print(url)
                urllib.request.urlopen(url)
                
                break  
            
            
def scrape3(p_snap, p_ktask, p_lock):

    td_column = 4 # column 
    
    print(web[0])
    driver = webdriver('eager')
    driver.get(web[0])
    
    if p_ktask.value:
        print('bef quit 333')
        driver.quit()
        return 0
    else:    
        button = "//nav[@id='ctabstoday']//ul[@class='nav nav-tabs']//li[@id='nav-asia-tab']"
        tab_asia = driver.find_element(By.XPATH, button)
        tab_asia.click()
        
        rows = "//tbody/tr"
        path = driver.find_elements(By.XPATH, rows)
        
        for row in path:
            if p_ktask.value:
                print('row quit 3333')
                driver.quit()
                return 0
            col = row.find_elements(By.TAG_NAME, "td")
            bucket = [x.text for x in col]
            if bucket and bucket[1] in "Philippines":
                driver.quit()
                column = bucket[td_column]
            
                global url
            
                with p_lock:
                    p_snap.update({"activ":f"{column}"})
                        
                for h,v in p_snap.items():
                    url += f"{h}={v}&"
                url = url[:-1]
                
                print(url)
                urllib.request.urlopen(url)
                
                break  
       
       
def snapscrape(data, skip_outdate_check=False):      
    
    date_snapshot = data['nwdat']
    date_current = date.today().strftime('%B.%d')
    date_current = 'July.31' # debug purposes
    
    # check if data is outdated
    print(date.today().strftime('%B.%d'))
    print(date_snapshot)
    
    is_outdated = date_snapshot != date_current
    if is_outdated and not skip_outdate_check:
        return is_outdated, data
    is_outdated = False
    
    print('data that was recieved')
    print(data)
    global url
    url += f"cases={data['cases']}&death={data['death']}"
    url += f"&recov={data['recov']}&nwdat={data['nwdat']}"
    url += f"&nwcas={data['nwcas']}&nwdea={data['nwdea']}"
    url += f"&activ={data['activ']}&wkper={data['wkper']}"
    urllib.request.urlopen(url)

    return is_outdated, data
        
        
def webscrape(data_snapshot=None):
    mngr = Manager()
    p_lock = Lock()
    p_snap = mngr.dict()
    p_ktask = mngr.Value('i', False)
    
    print('main')

    process = [
        Process(target=scrape1, args=(p_snap, p_ktask, p_lock, data_snapshot)),
        Process(target=scrape2, args=(p_snap, p_ktask, p_lock)),
        Process(target=scrape3, args=(p_snap, p_ktask, p_lock))
    ]
    
    for p in process:
        p.start()
    
    for p in process:
        p.join()
    
    # if kill was not called because snapshot, current date, and repository
    # are not the same, dump the updated data
    if not p_ktask.value:
        print(p_snap)
        with open("snapshot.json", "w") as file:
            json.dump(p_snap.copy(), file, indent = 4)
    # if kill was called becuase snapshot and repository are the same
    # but not current date, read snapshot instead
    elif p_ktask.value and data_snapshot:
        skip_outdate_check = True
        snapscrape(data_snapshot, skip_outdate_check)
        

def main():
    address = url
    # load animation
    urllib.request.urlopen(url) 
    # check if snapshot exists
    if os.path.exists('snapshot.json'):
        with open('snapshot.json', 'r') as f:
            outdated, data = snapscrape(json.load(f))
            if outdated:
                webscrape(data)
    else:
        webscrape()    

    return address
    