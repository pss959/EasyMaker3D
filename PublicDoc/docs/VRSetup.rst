Setting up |appname| for VR
===========================

As mentioned :ref:`at the very beginning <welcome>`, |appname| can be set up to
work with a VR system in addition to the mouse and keyboard. Any combination of
input works simultaneously.

The application has been run successfully with an Oculus Quest 2 and an HTC
Vive (coincidentally the systems I own). Other systems that are supported by
`SteamVR <https://www.steamvr.com>`_ should also work. If not, `contact the
author <https://github.com/pss959>`_ for help.

General Set-Up
--------------

Regardless of your VR system, you need to first install `SteamVR
<https://www.steamvr.com>`_ on your computer. Follow their instructions and
make sure everything works ok.

Some VR experiences allow you or force you to walk around while using them.
That wouldn't make sense for this application, so it is designed to make
everything useful from one spot. If you're planning to use the VR mode, set it
up for a comfortable, fixed standing or seated position. Note that the scene
view is set up for sitting, so you may need to adjust your VR positioning for a
different height if you prefer to stand.

Platform-Specific Instructions
------------------------------

Windows
.......

.. admonition:: Bug:

   Unfortunately, there seems to be a bug in the OpenVR library that interfaces
   with SteamVR that prevents rendering to the VR headset. The application can
   still work in :ref:`hybrid mode <ug-vr-modes>`, meaning you can still use
   the controllers.

Mac
...

It is not clear whether VR will work at all on a Mac, due to the `lack of
official SteamVR support
<https://store.steampowered.com/news/app/250820/view/2216278054495230717>`_.


Linux
.....

If you are unable to see any rendering in your VR headset, you may need to
apply the following workaround::

    cd /usr/share/vulkan/icd.d
    sudo mv intel_icd.x86_64.json intel_icd.x86_64.json.disabled

Note that this must be done each time after the `mesa-vulkan-drivers` package
is installed or updated.

Device-Specific Instructions
----------------------------

Oculus Quest 2
..............

The Oculus Quest 2 headset must be connected via a wired or Wi-Fi connection,
since |appname| runs on the host machine, not the headset.

Windows
,,,,,,,

Connect the Quest headset using a Link cable and the Oculus PC app, or use the
Wi-Fi Oculus Air Link. (Instructions for setting either of these up are pretty
easy to find online).  You should be able to just run |appname| and have it
come up in VR. If that doesn't work, try running SteamVR first, then running
the application.

Linux
,,,,,

Since there is no Oculus Link software that runs on Linux (!), the only choice
right now is using `ALVR <https://github.com/alvr-org/ALVR>`_ to connect the
Quest over Wi-Fi. Unfortunately, this is much more complicated than it should
be.

Initial Set-Up
::::::::::::::

 - Install ALVR on your computer.
 - ALVR replaces the compositor application used by SteamVR, so save the real
   version::

     cd ~/.steam/debian-installation/steamapps/common/SteamVR/bin/linux64/
     ln -s vrcompositor vrcompositor.real

 - Install the ALVR client on the Quest using `SideQuest
   <https://sidequestvr.com/>_`.
 - Run the ALVR server.
 - Follow the ALVR instructions and run the client on the Quest.

You should then be able to run |appname| over Wi-Fi to the Quest.

Switching Devices
:::::::::::::::::

Unfortunately, ALVR does a bad job of cleaning up after itself to allow SteamVR
to use a different device. If you have a different VR system you want to use:

 - Rename the ALVR server directory so SteamVR does not access its drivers
   first. (Really, if it finds this server it will run it despite having other
   devices to use.)
 - Restore the real SteamVR compositor you changed above::

    cd ~/.steam/debian-installation/steamapps/common/SteamVR/bin/linux64/
    ln -sf vrcompositor.real vrcompositor

To switch back to using ALVR again:

 - Restore the server directory to its original name.
 - Switch to using ALVR's compositor as in the initial set-up.

HTC Vive
........

Windows
,,,,,,,

You should be able to just run |appname| and have it come up in VR. If that
doesn't work, try running SteamVR first, then running the application.

Linux
,,,,,

Run SteamVR first, then run |appname|.
