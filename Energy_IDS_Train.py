import pandas as pd
import numpy as np
import joblib
import mysql.connector
from sklearn.ensemble import IsolationForest, RandomForestClassifier
from sklearn.model_selection import train_test_split
from sqlalchemy import create_engine, Column, Integer, Float, String, DateTime
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, declarative_base

# Configure database connection
db_config = {
    'host': 'localhost',
    'user': 'root',
    'password': '',
    'database': 'db_esp32'
}

# Create engine and session
engine = create_engine('mysql+mysqlconnector://root:@localhost/db_esp32')
Session = sessionmaker(bind=engine)
session = Session()

# Create the Base class
Base = declarative_base()

# Define the ElectricityMonitor class
class ElectricityMonitor(Base):
    __tablename__ = 'electricity_monitor'  # Use your actual table name here
    data_id = Column(Integer, primary_key=True)
    device_id = Column(String(255))
    voltage = Column(Float)
    current = Column(Float)
    power = Column(Float)
    energy = Column(Float)
    frequency = Column(Float)
    power_factor = Column(Float)
    created_at = Column(DateTime)
    
# Fetch data from the database with only power > 0.5 and order by created_at in descending order
def fetch_data():
    query = session.query(ElectricityMonitor).where(ElectricityMonitor.power > 0.5).order_by(ElectricityMonitor.created_at.desc())
    df = pd.read_sql(query.statement, query.session.bind)
    return df

def get_deviceId():
    query = session.query(ElectricityMonitor.device_id).distinct()
    device_ids = pd.read_sql(query.statement, query.session.bind)
    return device_ids

df = fetch_data()
device_ids = get_deviceId()

# Fill missing values with median
df['voltage'].fillna(df['voltage'].median())
df['current'].fillna(df['current'].median())
df['power'].fillna(df['power'].median())
df['energy'].fillna(df['energy'].median())
df['frequency'].fillna(df['frequency'].median())
df['power_factor'].fillna(df['power_factor'].median())
# Fill device_id with the latest id
latest_id = device_ids['device_id'].iloc[-1]
df['device_id'].fillna(latest_id)
# Fill created_at with current date and time
df['created_at'].fillna(pd.to_datetime('now'))

# Split the data into training and testing
train_df, test_df = train_test_split(df, test_size=0.33, random_state=42)


def train_if():
    for device_id in device_ids['device_id']:
        features = ['voltage', 'current', 'power', 'energy', 'frequency', 'power_factor']
        # Train model
        if_model = IsolationForest(contamination=0.1, random_state=42)
        if_model.fit(train_df[train_df['device_id'] == device_id][features])

        # Predict Label
        df.loc[df['device_id'] == device_id, 'if_label'] = if_model.predict(df[df['device_id'] == device_id][features])
        
        # Explicitly cast the 'if_label' column to string type right after prediction
        df['if_label'] = df['if_label'].astype(str)
        df.loc[df['device_id'] == device_id, 'if_label'] = np.where(df.loc[df['device_id'] == device_id, 'if_label'] == '1.0', 'normal', 'anomaly')

        filename = f'if_model_{device_id.replace(":", "-")}.pkl'
        joblib.dump(if_model, filename)
    

def train_rf():
    # Split the data into training and testing again for RF 
    train_df, test_df = train_test_split(df, test_size=0.33, random_state=42)

    for device_id in device_ids['device_id']:
        features = ['voltage', 'current', 'power', 'energy', 'frequency', 'power_factor']
        X = train_df[train_df['device_id'] == device_id][features]
        y = train_df[train_df['device_id'] == device_id]['if_label']
        
        # Ensure there is data to train on
        if not X.empty and not y.empty:
            rf_model = RandomForestClassifier(n_estimators=100, random_state=42)
            rf_model.fit(X, y)

            filename = f'rf_model_{device_id.replace(":", "-")}.pkl'
            joblib.dump(rf_model, filename)
            
            # Getting and printing feature importances
            feature_importances = rf_model.feature_importances_
            print(f"Feature importances for device {device_id}:")
            for feature, importance in zip(features, feature_importances):
                print(f"{feature}: {importance:.4f}")
        else:
            print(f"No data available for device {device_id}, skipping...")

train_if()
train_rf()

