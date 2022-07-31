import time

start_time = time.time()

import TRACKER as tck
import PROTOTYPE as ptp
import websocket 
import urllib

# connect to websocket server
ws = websocket.WebSocket()

while True:
    try:
        ws.connect('ws://192.168.1.12:81')
        print('connected to websocket server')
        ws.send('CLOSE_SOCKET')
        # wait for server to respond
        result = ws.recv()
        print("Recieved:", result)
        break
    except (TimeoutError, ConnectionRefusedError) as e:
        print(f'{e}')
        print('timeout..')
        time.sleep(1)
    
print('__WEBSOCKET LISTEN COMPLETE__')
print('proceeding to webscrape...')

if __name__ == '__main__':
    address = tck.main()
    print("--- %s seconds ---" % (time.time() - start_time))
    session = ptp.main()  
    if session:
        urllib.request.urlopen(address)

