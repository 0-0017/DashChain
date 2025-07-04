from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from Trainer import load_data
import numpy as np

app = FastAPI()

class Input(BaseModel):
    total_supply: float
    circ_supply: float
    balance: float
    votes: float
    height: float
    tx_volume: float

@app.post("/predict/")
def get_prediction(data: Input):
    input_dict = data.model_dump()
    result = load_data(input_dict)
    return {"prediction": result}

@app.get("/health")
def health_check():
    return {"status": "ok"}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run("main:app", host="127.0.0.1", port=8000, reload=True)