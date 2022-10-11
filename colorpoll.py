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
SAMPLE_SPREADSHEET_ID = '1RCKMWJJETZgv-eJU0g1uXDYhWpjO7LzYLGfcjcOhkVM'
SAMPLE_RANGE_NAME = 'Form Responses 1!A1:D'


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
        
        while keepgoing:
            time.sleep(1)
            result = sheet.values().get(spreadsheetId=SAMPLE_SPREADSHEET_ID,
                                      range=SAMPLE_RANGE_NAME).execute()
            values = result.get('values', [])
            if not values:
                print('No data found.')
                return
            
            # get the last row
            lastRow = values[-1]
            red = max(int(lastRow[1])*32-1,0)
            green = max(int(lastRow[2])*32-1,0)            
            blue = max(int(lastRow[3])*32-1,0)
            
            print("r%s g%s b%s" % (red,green,blue))
            newColorInt = (red << 16) | (green << 8) | (blue)
            print(hex(newColorInt))
            newColorStr = hex(newColorInt)[2:]
            if len(newColorStr) < 6:
            	prependZeroes = ["0" for i in range(6-len(newColorStr))]
            	newColorStr = "".join(prependZeroes) + newColorStr
            if newColorStr != desiredColor:
                print("New Color! " + newColorStr)
                desiredColor = newColorStr
                lastRequestedStamp = time.time()
                lastShownRed = red
                lastShownGreen = green
                lastShownBlue = blue
                currentColorStr = newColorStr
                os.system("particle call lightshow show " + currentColorStr)
    except HttpError as err:
        print(err)


if __name__ == '__main__':
    main()
