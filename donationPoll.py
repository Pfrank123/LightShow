from __future__ import print_function

import os.path
import time

from google.auth.transport.requests import Request
from google.oauth2.credentials import Credentials
from google_auth_oauthlib.flow import InstalledAppFlow
from googleapiclient.discovery import build
from googleapiclient.errors import HttpError

# If modifying these scopes, delete the file token.json.
SCOPES = ['https://www.googleapis.com/auth/spreadsheets.readonly']

# The ID and range of a sample spreadsheet.
SAMPLE_SPREADSHEET_ID = '1bn6EYSsHO_LtE1gwTvxgm5BxtR3q3FEEGzt-utsEydU'
SAMPLE_RANGE_NAME = 'Form Responses 1!A1:E'
TABLE_MAP_RANGE_NAME = 'TableMap!A2:B'
BEHAVIOR_RANGE_NAME = 'Behaviors!A2:E'


def main():
    """Shows basic usage of the Sheets API.
    Prints values from a sample spreadsheet.
    """
    
    
    creds = None
    # The file token.json stores the user's access and refresh tokens, and is
    # created automatically when the authorization flow completes for the first
    # time.
    if os.path.exists('token.json'):
        creds = Credentials.from_authorized_user_file('token.json', SCOPES)
    # If there are no (valid) credentials available, let the user log in.
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            flow = InstalledAppFlow.from_client_secrets_file(
                'credentials.json', SCOPES)
            creds = flow.run_local_server(port=0)
        # Save the credentials for the next run
        with open('token.json', 'w') as token:
            token.write(creds.to_json())

    try:
        service = build('sheets', 'v4', credentials=creds)

        # Call the Sheets
        sheet = service.spreadsheets()
        keepgoing=True
        desiredColor = ""
        lastRequestedStamp = time.time()
        lastShownStamp = time.time()
        lastShownRed = 0
        lastShownGreen = 0
        lastShownBlue = 0

        
        tableMapping = readTableMapFromSheet(service,sheet)
        bidLevelsToBehaviors = getAmountDataFromSheet(service,sheet)
        bidLevels = (list(bidLevelsToBehaviors.keys()))
        bidLevels.sort(reverse=True)
        
        lastShownIndex = 1
        with open('lastrow.txt', 'r') as lastrow:
            file_contents = lastrow.read()
            lastShownIndex = int(file_contents)
            print(f"reset lastShownIndex {lastShownIndex}")
            print(file_contents)
        
        while keepgoing:
            time.sleep(1.05)
            result = sheet.values().get(spreadsheetId=SAMPLE_SPREADSHEET_ID,
                                      range=SAMPLE_RANGE_NAME).execute()
            values = result.get('values', [])
            #print(lastShownIndex)
            if not values:
                print('No data found.')
                return

            while lastShownIndex < len(values):
                # get the next row
                activeRow = values[lastShownIndex]
                tablenumber = activeRow[3]
                pledgeAmount = int(activeRow[4])
                with open('lastrow.txt', 'w') as lastrow:
                    lastrow.write(str(lastShownIndex))
                
                print(pledgeAmount)
                for level in bidLevels:
                    if float(pledgeAmount) > level:
                        tableNum = tableMapping[tablenumber]
                        behavior = bidLevelsToBehaviors[level][0]
                        color = bidLevelsToBehaviors[level][1]
                        duration = bidLevelsToBehaviors[level][2]
                        arguments = bidLevelsToBehaviors[level][3]
                        command = f'''Particle call {tableNum} {behavior} "{color} {duration} {arguments}"'''
                        print(command)
                        os.system(command)
                    
                lastShownIndex += 1
    except HttpError as err:
        print(err)
        
def readTableMapFromSheet(service,sheet):
    query = sheet.values().get(spreadsheetId=SAMPLE_SPREADSHEET_ID,
                                  range=TABLE_MAP_RANGE_NAME).execute()
    rows = query.get('values', [])
    if not rows:
        raise('No config data found.')

    result = {}

    for row in rows:
        result[row[0]] = row[1]

    return result
def getAmountDataFromSheet(service,sheet):
    query = sheet.values().get(spreadsheetId=SAMPLE_SPREADSHEET_ID,
                                  range=BEHAVIOR_RANGE_NAME).execute()
    rows = query.get('values', [])                
    if not rows:
        if not rows:
            raise('No amount config found.')
            
    result = {}
    
    for row in rows:
    #loads the list
        result[int(row[0])] = row[1:]
    return result
    
    
    

if __name__ == '__main__':
    main()

