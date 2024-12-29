import serial
import time
import csv
from tkinter import Tk, Label, Button, StringVar
import winsound  # For alarm sound on Windows (you can modify this for other OS)

# ===== Step 1: Setup =====
# Configure serial communication (Replace "COM3" with your Arduino's port)
arduino = serial.Serial(port="COM3", baudrate=9600, timeout=1)

# Initialize CSV file
csv_file = "sensor_data.csv"
try:
    # Create the file with headers if it doesn't exist
    with open(csv_file, "x", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(["distance", "temperature", "humidity", "alarm"])
except FileExistsError:
    pass

# ===== Step 2: GUI Configuration =====
# Setup the GUI
root = Tk()
root.title("Arduino AI/ML Monitoring System")

# Variables to update GUI labels
distance_var = StringVar()
temperature_var = StringVar()
humidity_var = StringVar()
alarm_var = StringVar()

# Set default text
distance_var.set("Distance: -- cm")
temperature_var.set("Temperature: -- °C")
humidity_var.set("Humidity: -- %")
alarm_var.set("Alarm: --")


# ===== Step 3: Functions =====
def read_from_arduino():
    """Read sensor data from Arduino and save it to a CSV file."""
    if arduino.in_waiting > 0:
        try:
            # Read data and decode
            data = arduino.readline().decode().strip()
            # Expected format: distance,temperature,humidity,alarm
            distance, temperature, humidity, alarm = map(float, data.split(","))
            
            # Save data to CSV
            with open(csv_file, "a", newline="") as file:
                writer = csv.writer(file)
                writer.writerow([distance, temperature, humidity, alarm])
            
            return distance, temperature, humidity, alarm
        except ValueError:
            # Skip malformed lines
            return None
    return None


def check_alarm_conditions(temperature, humidity):
    """Check if the alarm should be triggered based on temperature and humidity."""
    # Set temperature and humidity thresholds
    TEMPERATURE_THRESHOLD = 30  # Temperature threshold in °C
    HUMIDITY_THRESHOLD = 70     # Humidity threshold in %

    # Trigger alarm if temperature exceeds threshold or humidity exceeds threshold
    if temperature > TEMPERATURE_THRESHOLD or humidity > HUMIDITY_THRESHOLD:
        return True
    return False


def update_gui():
    """Update GUI labels with real-time sensor data."""
    data = read_from_arduino()
    if data:
        distance, temperature, humidity, alarm = data

        # Check if alarm conditions are met
        if check_alarm_conditions(temperature, humidity):
            # Trigger alarm sound (Windows example)
            winsound.Beep(1000, 500)  # Frequency: 1000 Hz, Duration: 500 ms
            alarm = 1  # Set alarm active if condition is met
        else:
            alarm = 0  # Set alarm inactive if conditions are normal

        # Update GUI with real-time data
        distance_var.set(f"Distance: {distance:.2f} cm")
        temperature_var.set(f"Temperature: {temperature:.2f} °C")
        humidity_var.set(f"Humidity: {humidity:.2f} %")
        alarm_var.set(f"Alarm: {'Active' if alarm == 1 else 'Inactive'}")
    # Schedule the next update
    root.after(100, update_gui)  # Refresh every 100 ms


def reset_alarm():
    """Send a reset command to Arduino."""
    try:
        arduino.write(b"RESET\n")  # Send reset command
        time.sleep(0.1)  # Allow Arduino to process
        alarm_var.set("Alarm: Resetting...")
    except Exception as e:
        alarm_var.set(f"Error: {str(e)}")


# ===== Step 4: GUI Layout =====
# Add labels to the GUI
Label(root, textvariable=distance_var).pack()
Label(root, textvariable=temperature_var).pack()
Label(root, textvariable=humidity_var).pack()
Label(root, textvariable=alarm_var).pack()
Button(root, text="Reset Alarm", command=reset_alarm).pack()

# ===== Step 5: Start the GUI =====
update_gui()
root.mainloop()
