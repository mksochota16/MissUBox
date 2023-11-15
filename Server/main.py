import os
from datetime import datetime
from typing import List

import uvicorn as uvicorn
from fastapi import FastAPI, HTTPException, status
from starlette.middleware.cors import CORSMiddleware

from Server.models import ArduinoResponse, ArduinoAction, ActionType, ActionsPostBody

API_KEY = "123" # input your API key here or read it from env
ACTIONS_BUFFER_FILE = "buffer_file.txt"
LOG_FILE = "log_file.txt"
STATUS_FILE = "status_file.txt"

app = FastAPI()
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Adjust this to restrict origins if needed
    allow_methods=["*"],  # Adjust this to restrict HTTP methods if needed
    allow_headers=["*"],  # Adjust this to restrict headers if needed
)

@app.get("/",
         response_model=ArduinoResponse)
async def get_actions(api_key: str):
    if api_key != API_KEY:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Incorrect API KEY"
        )
    with open(STATUS_FILE, 'w') as file:
        file.write(f"{datetime.now()}")

    if os.path.getsize(ACTIONS_BUFFER_FILE) == 0:
        return ArduinoResponse(
            actions = []
        )

    read_data_list = []
    current_dict = {}
    with open(ACTIONS_BUFFER_FILE, "r") as file:
        # Read lines from the file and convert them back to a list of dictionaries (deserialize)
        for line in file:
            if line.strip() == "=====":
                # A separator indicates the end of a dictionary, so add the current_dict to the list
                read_data_list.append(current_dict)
                current_dict = {}
            else:
                try:
                    key, value = line.strip().split(": ")
                    current_dict[key] = value
                except ValueError:
                    current_dict["value"] = ""

    # clear buffer file
    with open(ACTIONS_BUFFER_FILE, "w") as file:
        pass

    # write datetime of last request
    with open(LOG_FILE, "a+") as file:
        file.write(f"<-READ {datetime.now()}\n")

    actions: List[ArduinoAction] = [ArduinoAction(type=ActionType[action["type"].split('.')[1]], value=action["value"]) for action in read_data_list]
    return ArduinoResponse(
        actions = actions
    )


@app.post("/")
async def post_actions(api_key: str, actions: ActionsPostBody):
    if api_key != API_KEY:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Incorrect API KEY"
        )
    text = ""
    with open(ACTIONS_BUFFER_FILE, "w") as file:
        # Write the dictionary to the file as a string (serialize)
        for action in actions.actions:
            for key, value in action.model_dump().items():
                file.write(f"{key}: {value}\n")
                if key == 'type' and value == ActionType.TEXT:
                    text = value
            file.write("=====\n")

    # write datetime of last request
    with open(LOG_FILE, "a+") as file:
        file.write(f"->SENT {datetime.now()} {text}\n")

    return {"message": "ok"}

@app.get("/status")
async def get_status(api_key: str):
    if api_key != API_KEY:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Incorrect API KEY"
        )

    with open(STATUS_FILE, "r+") as file:
        request_datetime = file.readline()

    return {"last_request_time": request_datetime}


@app.get("/logs")
async def get_logs(api_key: str):
    if api_key != API_KEY:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Incorrect API KEY"
        )
    logs = []
    with open(LOG_FILE, 'r+') as file:
        logs = file.readlines()

    return {"logs": logs}


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=80)

