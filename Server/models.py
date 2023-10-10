from enum import Enum
from typing import List, Optional

from pydantic import BaseModel

class ActionType(Enum):
    TEXT = 0
    SPIN = 1

class ArduinoAction(BaseModel):
    type: ActionType
    value: str = "" #max 31 characters
class ArduinoResponse(BaseModel):
    actions: List[ArduinoAction]

