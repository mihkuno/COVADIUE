import urllib.request
from time import sleep
from selenium.webdriver import Chrome
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.service import Service
from webdriver_manager.chrome import ChromeDriverManager

import time
start_time = time.time()

# start display animation 3.2 seconds
url =  "http://192.168.1.6/covscrape"
urllib.request.urlopen(url) 

# chromium driver library
driver = ChromeDriverManager().install()
driver = Chrome(service=Service(driver))

# https://doh.gov.ph/covid19tracker
# bed occupancy 
# number of facilities
# region/province scope
# equipment availability 
# occupied and vacancies

# target secondhand source
driver.get('https://www.worldometers.info/coronavirus/country/philippines/')


# extract data
tcases = driver.find_element(By.XPATH, "//div[@class='content-inner']//div[4]").text[19:]
tdeath = driver.find_element(By.XPATH, "//div[@class='content-inner']//div[5]").text[8:]
trecov = driver.find_element(By.XPATH, "//div[@class='content-inner']//div[6]").text[11:]
mupdat = driver.find_element(By.XPATH, "//div[@id='news_block']").text.splitlines()

# updates
ccdate = mupdat[1][:-6].replace(" ", ".")

ndeath = mupdat[3][mupdat[3].index('and')+5:mupdat[3].index('new deaths')-1] if 'new deaths' in mupdat[3] else "0"
ncases = mupdat[3][:mupdat[3].index('new cases')-1] if 'new cases' in mupdat[3] else "0"

print(f"Cases: {tcases}") # total cases
print(f"Death: {tdeath}") # total death
print(f"Recov: {trecov}") # total recov
print(f"Curnt: {ccdate}") # latest date  
print(f"Newca: {ncases}") # latest cases
print(f"Newde: {ndeath}") # latest death
print("__________________\n")

url += f"?cases={tcases}&death={tdeath}"
url += f"&recov={trecov}&nwdat={ccdate}"
url += f"&nwcas={ncases}&nwdea={ndeath}"

urllib.request.urlopen(url)

print("--- %s seconds ---" % (time.time() - start_time))
