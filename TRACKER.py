import urllib.request
from selenium.webdriver import Chrome
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.service import Service
from webdriver_manager.chrome import ChromeDriverManager

import time
start_time = time.time()

# start display animation
url =  "http://192.168.1.12/covscrape"
urllib.request.urlopen(url) 

web = [
    'https://www.worldometers.info/coronavirus/#main_table',
    'https://www.worldometers.info/coronavirus/country/philippines/',
    'https://www.worldometers.info/coronavirus/weekly-trends/#weekly_table',
]

# chromium driver library
driver = ChromeDriverManager().install()
driver = Chrome(service=Service(driver))

# https://doh.gov.ph/covid19tracker
# bed occupancy 
# number of facilities
# region/province scope
# equipment availability 
# occupied and vacancies

## NATIONWIDE GENERAL DATA
## --------------------------------------------------------------------------------------
## --------------------------------------------------------------------------------------

driver.get(web[1])

# extract data
tcases = driver.find_element(By.XPATH, "//div[@class='content-inner']//div[4]").text[19:]
tdeath = driver.find_element(By.XPATH, "//div[@class='content-inner']//div[5]").text[8:]
trecov = driver.find_element(By.XPATH, "//div[@class='content-inner']//div[6]").text[11:]
mupdat = driver.find_element(By.XPATH, "//div[@id='news_block']").text.splitlines()

# updates
ccdate = mupdat[1][:-6].replace(" ", ".")
ndeath = mupdat[3][mupdat[3].index('and')+5:mupdat[3].index('new deaths')-1] if 'new deaths' in mupdat[3] else "0"
ncases = mupdat[3][:mupdat[3].index('new cases')-1] if 'new cases' in mupdat[3] else "0"

url += f"?cases={tcases}&death={tdeath}"
url += f"&recov={trecov}&nwdat={ccdate}"
url += f"&nwcas={ncases}&nwdea={ndeath}"
urllib.request.urlopen(url)

# print(f"Cases: {tcases}") # total cases
# print(f"Death: {tdeath}") # total death
# print(f"Recov: {trecov}") # total recov
# print(f"Curnt: {ccdate}") # latest date  
# print(f"Newca: {ncases}") # latest cases
# print(f"Newde: {ndeath}") # latest death
# print(f"Activ: {active}") # active cases
# print(f"wkper: {wkperc}") # weekly %change


## NATIONWIDE ACTIVE AND WEEKLY DATA
## --------------------------------------------------------------------------------------
## --------------------------------------------------------------------------------------

def get(url, id):
    driver.get(url)

    tab_asia = driver.find_element(By.XPATH, "//nav[@id='ctabstoday']//ul[@class='nav nav-tabs']//li[@id='nav-asia-tab']")
    tab_asia.click()

    path = driver.find_elements(By.XPATH, "//tbody/tr")
    for row in path:
        col = row.find_elements(By.TAG_NAME, "td")
        bucket = [x.text for x in col]
        if bucket and bucket[1] in "Philippines":
            column = bucket[id]
            return column
        

col_1 = 8 # column
col_2 = 4 # column 

active = get(web[0], col_1)
wkperc = get(web[2], col_2)
print(f'Active Cases: {active}')
print(f'Percent Change: {wkperc}')

driver.close()

# convert url symbols
wkperc = wkperc.replace("+","%2B") if "+" in wkperc else wkperc.replace("-",'%2D')    

url += f"&activ={active}&wkper={wkperc}"
urllib.request.urlopen(url)

print("--- %s seconds ---" % (time.time() - start_time))