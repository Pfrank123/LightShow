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

        lastShownIndex = 1
        while keepgoing:
            time.sleep(0.5)
            result = sheet.values().get(spreadsheetId=SAMPLE_SPREADSHEET_ID,
                                      range=SAMPLE_RANGE_NAME).execute()
            values = result.get('values', [])
            if not values:
                print('No data found.')
                return

            while lastShownIndex < len(values):
                # get the next row
                activeRow = values[lastShownIndex]
                tablenumber = activeRow[3]
                pledgeAmount = int(activeRow[4])

                if pledgeAmount >= 100:
                    #b = 1

                    os.system("particle call " + tableMapping[tablenumber] + " show 03fccf")
                    '''
                    os.system("particle call " + tableMapping[tablenumber] + " show1 5f0ca8")
                    time.sleep(0.4)
                    os.system("particle call " + tableMapping[tablenumber] + " show 5f0ca8")
                    #b+=1
                        '''

                if pledgeAmount < 100:
                    os.system("particle call " + tableMapping[tablenumber] + " show 00FF00")


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


if __name__ == '__main__':
    main()

