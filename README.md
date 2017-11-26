# The Mousetracker Browser
The Mousetracker Browser is an extended version of the Qt example using the QWebEngine.

## Introduction
The added tweaks on this browser make it capable of tracking user cursor movements in sessions and produce screenshots with detailed mouse trails of the traced web pages.
The screenshots are captured of the whole web page on session ending i.e. when the user closes the tab or on successful navigation requests i.e. when the user clicks through hyperlinks or on request by hitting the save image button in the address bar (old floppy disk).

## Features
The Mousetracker Browser offers:
<ul>
<li> An adaption for capturing the fixed elements on scrollable pages with their mouse recordings on a screenshot. </li>
<li> A full screenshot of the web page that is viewed by the user with their mouse interactions. </li>
<li> The ability to analyze the users' mouse moves on the web page. </li>
</ul>


## Motivation

## How to set and run the tweaked demobrowser source code:
<ul>
<li> Download and install QT 5.8 (will also download the creator IDE) with MSVC2015:
<ul>
	<li> Offline: "https://www.qt.io/download-open-source/#section-2" </li>
	<li> Online: "https://www.qt.io/download-open-source/" </li>
	<li> Follow the installation, check QT5.8 with MSVC2015-32bit compiler </li>
</ul>
</li>
<br>
<li> For a debugger among with the MSVC2015 compiler, CDB can be used from the Windows SDK:
<ul>
	<li> Download Windows SDK from "https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk" </li>
	<li> Follow the installation wizard until the installation choices. Only the debugging tools choice is needed </li>
</ul>
</li>
<br>
<li> Copy RC commands:
<ul>
	<li> Navigate to: 
		"C:\Program Files (x86)\Windows Kits\8.1\bin\x86" </li>
	<li> Copy:
		"rc.exe" and "rcdll.dll" </li>
	<li> Paste in the following directory:
		"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC" </li>
</ul>
</li>
<br>
<li> Download OpenSSL for enabling https browsing:
<ul>
	<li> Download from "http://p-nand-q.com/programming/windows/building_openssl_with_visual_studio_2013.html" </li>
	<li> Version "OpenSSL 1.0.2j" under "Visual Studio 2015", choose "32-Bit Release DLL" </li>
	<li> Unzip the archeive into a known directory </li>
</ul>
</li>
<br>
<li> Edit the environment variables for the project:
<ul>
	<li> Add the directory of the bin folder of the OpenSSL to the PATH (e.g C:\openssl\bin) </li> 
</ul>
</li>
