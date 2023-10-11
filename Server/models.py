from enum import Enum
from typing import List, Optional

from pydantic import BaseModel

class ActionType(Enum):
    TEXT = 0
    SPIN = 1

class ArduinoAction(BaseModel):
    type: ActionType
    value: str = "" #max 31 characters

class MessageAction(ArduinoAction):
    type: ActionType = ActionType.TEXT
    @classmethod
    def create(cls, text: str):
        if len(text) < 32:
            return cls(value=text)

class SpinAction(ArduinoAction):
    type: ActionType = ActionType.SPIN
    @classmethod
    def create(cls):
        return cls()
class ArduinoResponse(BaseModel):
    actions: List[ArduinoAction]

class ActionsPostBody(ArduinoResponse):
    pass

