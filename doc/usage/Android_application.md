# Android application usage

Once the application was installed, if you start you should see the main screen:

![Main page of the Android application](img/main_page.jpg | height=400)

On the top you can see the button that open the setting page:

![Main page of the Android application with highlighted the setting button on the top right](img/main_page_settings_highlight.jpg | height=400)

The first time it is necessary to set or regulate inside the settings of each phone that participate to the session, a series of parameters:

- **Relay Address**: The IP address of the server where the MLAPI-docker relay is located. You can discovery it, by running on the server the command ```ip address```. It can be also a DNS entry
- **Elaboration Server Address**: The IP address of the server where the elaboration server is located. You can discovery it, by running on the server the command ```ip address```. It can be also a DNS entry
- **FPS**: The number of frame per second to acquire during the session
- **Auto Stop After N sec**: The number of second to wait before the host automatically stop the session. It is useful when it is necessary to make some test and we want to acquire a set amount of frame between sessions
- **Ping Compensation**: Enable disable the latency compensation system, we suggest to leave it on.
- **Reserved bandwidth**: How much bandwidth must be reserved to the trigger sending (for more detail about this aspect please reference to the paper)
- **Start delay**: How much time wait after the recording starting button is pressed to really staring acquiring the session (useful mostly for testing)

Once you setup the paprameters, you can press the done button at the bottom of the screen.

![Settings page of the Android application](img/settings.jpg | height=400)

To create a new session the host need to press the plus sign on the bottom right:

![Main page of the Android application with highlighted the new session button on the bottom right](img/main_page_new_session_highlight.jpg | height=400)

Now the other clients can join the created session using the ```join``` button, if the session is not shown it is possible to refresh the list using the button on bottom left

![Main page of the Android application with highlighted the refresh list button on the bottom right](img/main_page_refresh_highlight.jpg | height=400)

Once inside the session each client start mapping the scene, once the scene is fully mapped and all the people are entered, the host can place the anchor by pressing to a plane.

Once placed after few seconds the recording button will be shown on the screen.

![Example of the view before the recording started](img/recording_started.jpg | height=400)

To end the recording the host must press the stop button

![Example of the view with the stop button](img/recording_in_progress.jpg | height=400)