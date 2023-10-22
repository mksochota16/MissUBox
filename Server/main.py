from typing import List

import uvicorn as uvicorn
from fastapi import FastAPI, HTTPException, status
from starlette.middleware.cors import CORSMiddleware

from Server.models import ArduinoResponse, ArduinoAction, ActionType, ActionsPostBody

API_KEY = "123" # input your API key here or read it from env

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

    read_data_list = []
    current_dict = {}
    with open("buffer_file.txt", "r") as file:
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
    with open("buffer_file.txt", "w") as file:
        pass

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

    with open("buffer_file.txt", "w") as file:
        # Write the dictionary to the file as a string (serialize)
        for action in actions.actions:
            for key, value in action.model_dump().items():
                file.write(f"{key}: {value}\n")
            file.write("=====\n")

    return {"message": "ok"}


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=80)

