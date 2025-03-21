import pyttsx3
import speech_recognition as sr
import re
import time
import serial

# Initialize the text-to-speech engine (pyttsx3)
engine = pyttsx3.init()
try:
    # Try to establish a serial connection
    arduino = serial.Serial('COM13', 9600, timeout=1)
    print("Successfully connected to Arduino on COM13")
except serial.SerialException:
    print("Error: Unable to connect to Arduino. Please check the connection or port.")
except Exception as e:
    print(f"An unexpected error occurred: {e}") 
time.sleep(2) 

# Set properties for speech output (optional)
engine.setProperty('rate', 150)  # Speed of speech
engine.setProperty('volume', 0.9)  # Volume level (0.0 to 1.0)

# Set a robotic-sounding voice (if available on the system)
voices = engine.getProperty('voices')
robot_voice = voices[1]  # Select a mechanical or synthetic voice (this may vary per system)
engine.setProperty('voice', robot_voice.id)

# Global variable to track timer state
timer_active = False
timer_end_time = None  # Track the time when the timer ends

# Define the prompt for LittleChef
prompt = """
You are LittleChef, a friendly and supportive cooking assistant. Always give one very small step at a time and wait for a response before moving forward.
Always check if they have the ingredients (one at a time, this is important). If they need to weigh ingredients, give the quantity and suggest they use your scale (you have one inbuilt).
Every time you mention a timer, you should include a tag like /TIMER3/ to represent a timer for 3 minutes.
Do not ask the user to tell you when something is cooked; either give them another task or say, "Now we wait for it to finish cooking, you can relax."
If the user asks about something unrelated to cooking, politely redirect them back to the recipe with a positive tone.

Don’t give too much information at once. Allow the user time to respond before continuing.
"""

def listen_to_user():
    recognizer = sr.Recognizer()
    with sr.Microphone() as source:
        print("\nListening for your command... (Say something or stay silent)")
        recognizer.adjust_for_ambient_noise(source)
        audio = recognizer.listen(source)

    try:
        user_text = recognizer.recognize_google(audio)
        return user_text

    except sr.UnknownValueError:
        return None  

    except sr.RequestError:
        error_message = "Sorry, there was an issue with the speech recognition service."
        print(error_message)
        speak_response(error_message)
        return None

# Function to simulate AI response manually
def get_littlechef_response(user_input):
    print("\n Respond to: ", user_input)
    response = input("\nEnter LittleChef's response: ")  # Manually input a response
    return response.strip()

# Function to speak the assistant's response (text-to-speech)
def speak_response(response_text):
    clean_text = re.sub(r"/TIMER\d+/", "", response_text).strip()
    # Directly speak the response (without queue)
    engine.say(clean_text)
    engine.runAndWait()

# Function to parse and handle tags (timers and item labels)
def handle_tags(response_text):
    # Pattern to match the timer tag (e.g., /TIMER3/)
    timer_pattern = r"/TIMER(\d+)/"
    
    timer_match = re.search(timer_pattern, response_text)  # Find timer
    
    if timer_match:
        minutes = int(timer_match.group(1))  # Extract the number of minutes
        start_timer(minutes)  # Call the function to handle the timer

    return timer_match

def start_timer(minutes):
    global timer_active, timer_end_time
    print(f"\nTimer for {minutes} minutes started")
    timer_active = True
    timer_end_time = time.time() + (minutes * 1)  # Calculate the end time for the timer

def check_timer():
    """Check if the timer has ended"""
    global timer_active
    if timer_active and time.time() >= timer_end_time:
        timer_active = False
        return True  # Timer has finished
    return False

def check_for_exit(user_input):
    exit_keywords = ["exit", "quit", "stop", "goodbye"]
    return any(word in user_input.lower() for word in exit_keywords)

# Function to simulate a timer
def main():
    print("\nWaiting for the user to say 'hello', 'hi', or 'Little Chef'...")  

    print(prompt)  # Display the prompt in the terminal

    # Simulate sending prompt (manual response for now)
    littlechef_intro = get_littlechef_response("Introduce yourself to the user.")
    speak_response("Hi! I’m LittleChef, your step-by-step cooking buddy. Let’s make something delicious together!")

    while True:
        if check_timer():
                    speak_response("The timer has finished!")
                    arduino.write(b'TIMER_DONE') 
                    print("Sent 'TIMER_DONE' to Arduino")
        user_input = listen_to_user()
        if user_input:
            # Check if the user wants to exit
            if check_for_exit(user_input):
                speak_response("Goodbye! Happy cooking!")
                break  # Exit the loop if the user said "exit" or similar

            print(f"\nUser: {user_input}")  # Show user input

            if check_timer():
                    speak_response("The timer has finished!")
                    arduino.write(b'TIMER_DONE') 
                    print("Sent 'TIMER_FINISHED' to Arduino")
            
            littlechef_response = get_littlechef_response(user_input)  # Simulated LLM response

            # If LittleChef's response contains a timer tag, we handle the timer
            if littlechef_response:
                handle_tags(littlechef_response)  # Handle timers if needed
                
                # Before speaking, check if the timer has ended
                
                # Speak the response
                speak_response(littlechef_response)  # Speak the response

if __name__ == "__main__":
    main()
