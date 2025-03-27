import pyttsx3
import speech_recognition as sr
import re
import time
import serial
import openai  
import os  
import sys

apikey = ""
# (replace with your actual key)
openai.api_key = apikey


ASSISTANT_ID = "asst_aapUnANFXgPBNW6vnagt4v48"

# text-to-speech engine (pyttsx3)
engine = pyttsx3.init()
try:
    # make serial connection
    arduino = serial.Serial('COM14', 115200, timeout=1)
    print("Successfully connected to Arduino on COM1  ")
except serial.SerialException:
    print("Error: Unable to connect to Arduino. Please check the connection or port.")
except Exception as e:
    print(f"An unexpected error occurred: {e}") 
time.sleep(2) 

# speech stuff
engine.setProperty('rate', 180)  # speed of speech
engine.setProperty('volume', 1)  # volume (0-1) 

# Set a robotic-sounding voice 
voices = engine.getProperty('voices')
robot_voice = voices[1] 
engine.setProperty('voice', robot_voice.id)

# timer state to track
timer_active = False
timer_end_time = None  



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

# get little chef response
def get_littlechef_response(user_input, thread_id):
    try:
        # make a new convo thread
        openai.beta.threads.messages.create(
            thread_id=thread_id,
            role="user",
            content=user_input
        )

        # run assistant
        run = openai.beta.threads.runs.create(
            thread_id=thread_id,
            assistant_id=ASSISTANT_ID
        )
        run_id = run.id  

        # wait for response
        while True:
            run_status = openai.beta.threads.runs.retrieve(thread_id=thread_id, run_id=run_id)
            if run_status.status == "completed":
                break
            time.sleep(1)  

        # get reply
        messages = openai.beta.threads.messages.list(thread_id=thread_id)
        
        if messages.data:  #check if there's a response
            response_text = messages.data[0].content[0].text.value
            print(f"\n🔹 AI Response: {response_text}")  
            return response_text  
        else:
            print("\n No response received from OpenAI.")
            return "Sorry, I couldn't process that."

    except Exception as e:
        print(f"Error getting response from OpenAI Assistant: {e}")
        return "Sorry, I couldn't process that."


# speak the response
def speak_response(response_text):
    clean_text = re.sub(r"/TIMER\d+/", "", response_text).strip()
    engine.say(clean_text)
    engine.runAndWait()

# handle timer tag
def handle_tags(response_text):
    timer_pattern = r"/TIMER(\d+)/"
    
    timer_match = re.search(timer_pattern, response_text)  # Find timer tag
    
    if timer_match:
        minutes = int(timer_match.group(1))  # extract the number of minutes
        start_timer(minutes) # handle the timer with func

    return timer_match

def start_timer(minutes):
    global timer_active, timer_end_time
    print(f"\nTimer for {minutes} minutes started")
    timer_active = True
    timer_end_time = time.time() + (minutes * 3) # time when it ends (not the actual time)

def check_timer():
    """Check if the timer has ended"""
    global timer_active
    if timer_active and time.time() >= timer_end_time:
        timer_active = False
        return True  # Timer has finished
    return False

def check_for_exit(user_input):
    exit_keywords = ["exit", "quit", "goodbye", "bye"]
    return any(word in user_input.lower() for word in exit_keywords)


def main():
    print("Waiting for Arduino to send 'START' signal...")  

    while True:
        if arduino.in_waiting > 0:  # check if there's incoming data from Arduino
            signal = arduino.readline().decode(errors='ignore').strip()   # Read and decode the signal
            if signal == "START":
                print("Received 'START' signal from Arduino. Beginning LittleChef...")
                break  #start assistant



    # create a thead for convo
    thread = openai.beta.threads.create()
    thread_id = thread.id  
    # send start message
    littlechef_intro = get_littlechef_response("Introduce yourself to the user very breifly. tell the user that for todays demo you can walk them through a pasta dish with a cheesy bechamel sauce. also tell the user they can turn you off by saying goodbye. dont use emojis and sound like natural human speech.", thread_id)
    arduino.write(b'TIMER_DONE') 
    speak_response(littlechef_intro)


    while True:
        # check if timer is done
        if check_timer(): 
            littlechef_response = get_littlechef_response("tell the user the timer has finished!", thread_id)
            speak_response("Ding, ding")
            arduino.write(b'TIMER_DONE')
            if littlechef_response:
                handle_tags(littlechef_response)
                speak_response(littlechef_response)
            print("Sent 'TIMER_DONE' to Arduino")

        user_input = listen_to_user()
        if user_input:
            # check for exit
            if check_for_exit(user_input):
                speak_response("Goodbye! Happy cooking! You can always turn me on again by pressing my hat.")
                arduino.write(b'EXIT')
                print("Waiting for Arduino to send 'START' signal...")  
                # if start start again
                while True:
                    if arduino.in_waiting > 0:  # check if theres incoming from Arduino
                        signal = arduino.readline().decode(errors='ignore').strip()   
                        if signal == "START":
                            print("Received 'START' signal from Arduino. Beginning LittleChef...")
                            break  
              
                littlechef_intro = get_littlechef_response("Say that you are back and can help out to the user", thread_id)
                arduino.write(b'TIMER_DONE') 
                speak_response(littlechef_intro)
    
                continue


            print(f"\nUser: {user_input}")  # Show user input

            if check_timer():
                arduino.write(b'TIMER_DONE') 
                littlechef_response = get_littlechef_response("tell the user the timer has finished!", thread_id)
                if littlechef_response:
                    handle_tags(littlechef_response)
                    speak_response(littlechef_response)
                print("Sent 'TIMER_DONE' to Arduino")


            # remember convo
            littlechef_response = get_littlechef_response(user_input, thread_id)

            if littlechef_response:
                handle_tags(littlechef_response)
                speak_response(littlechef_response) 


if __name__ == "__main__":
    main()
