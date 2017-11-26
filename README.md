# A tweaked demobrowser
The demobrowser is a Qt example on the QWebEngine class.

## Introduction
The tweaked demobrowser is a full web browser with history, bookmarks and almost all other important functionalities of any web browser.
The added tweaks on this demobrowser make it capable of tracking user cursor movements on the web pages.
The results are captured in a static screenshot of the whole web page when the user closes the tab, hit the save image button or click through pages.

The tweaked demobrowser offers:
<ul>
<li> Analyzing mouse movements trails on the web page. </li>
<li> A full screenshot of the web page that is viewed by the user. </li>
<li> An adaption for the recordings on fixed elements on scrollable pages. </li>
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
<li> For a debugger among with the MSVC2015 compiler, CDB can be used from Windows SDK:
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
