# predict.py

import sys
import json
import pandas as pd
import joblib

# Function to predict anomalies
def predict_anomalies(new_data, device_id):
    features = ['voltage', 'current', 'power', 'energy', 'frequency', 'power_factor']
    X_new = new_data[features]
    
    # Load the models from disk
    if_model = joblib.load(f'../ids_models/if_model_{device_id.replace(":", "-")}.pkl')
    rf_model = joblib.load(f'../ids_models/rf_model_{device_id.replace(":", "-")}.pkl')
    
    # Use Isolation Forest to label the data
    if_labels = if_model.predict(X_new)
    # new_data['if_labels'] = if_labels
    # new_data['if_labels'] = new_data['if_labels'].map({1: 'normal', -1: 'anomaly'})  # Map 1 to 'normal' and -1 to 'anomaly'
    
    # Use Random Forest to make the final prediction
    rf_labels = rf_model.predict(X_new)
    new_data['label'] = rf_labels

    # Convert feature importances to a string or a list (to fit into a single cell)
    # new_data['feature_importances'] = [list(rf_model.feature_importances_)]
    
    # Round all float columns to 2 decimal places
    float_cols = new_data.select_dtypes(include=['float64']).columns
    new_data[float_cols] = new_data[float_cols].round(2)
    
    return new_data

# Read data from stdin
input_data = sys.stdin.read()
input_json = json.loads(input_data)

new_data = pd.DataFrame([input_json['data']])
device_id = input_json['device_id']

# Predict and print results
predictions = predict_anomalies(new_data, device_id)
print(predictions.to_json(orient='records'))
