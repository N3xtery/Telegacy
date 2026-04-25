<p align="center">
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="https://github.com/user-attachments/assets/47a2e686-bbc7-4a70-bf3b-1a951057abd7">
  <img src="https://github.com/user-attachments/assets/6fee1373-4920-44ae-947b-8387d6b0306a"/>
</picture>
</p>

# Telegacy
An unofficial Telegram client aimed to work on old versions of Windows (NT3.51/95/NT4/98/2000/ME/XP). Doesn't use an external Telegram API library of any kind. Written in C++ with pure Windows API and a few libraries (see below). A good portion of the Telegram features is already implemented.

## Features
- Logging in with a code sent to your account or a QR code
- 2FA support (it's a heavy process, at least Pentium 4 / Pentium M is needed to do it in time)
- Folders
- Message sending and receiving
- Microsoft Global IME support for Chinese/Japanese/Korean input on non-CJK versions of Windows
- Image previews
- Emojis
- Custom emojis (can't send them though)
- Stickers (only receiving)
- Message editing, deleting, forwarding, replying, reacting
- Text formatting
- File downloading and uploading
- Voice message recording
- Themes (just the color)
- Profiles
- Notifications
- SOCKS5 proxies support
- Help system

## Minimum system requirements
- Windows 95 RTM / Windows NT 3.51 RTM
- 486 CPU
- The client uses about 5-6 MB of RAM with disabled images and emojis, which, when enabled and many, can eat quite a bit of RAM, so account for that

## System support
If you want the best experience, you need to have at least comctl32.dll version 4.71 (comes with IE 4.0 or standalone) and, optionally, Crypto API support (comes with IE 3.02) for a more secure random numbers generator. If you have comctl32.dll <=4.70, you won't have the dropdown arrow for the files button on your toolbar (its functionality will be transfered to the "Show the uploading list" option in the Tools menu). In case of a comctl32.dll <=4.00 you won't get the flat toolbar style, and no date and time picker control for changing your birthday (which becomes no longer possible).

## Included DLLs
- unicows.dll 1.1.3790.0 for Windows 9x
- riched20.dll 5.30.23.1230
- msls31.dll 3.10.349.0
- msvcrt.dll 6.0.8397.0 for systems without one at all

## Libraries used
- libtomcrypt + libtommath, for cryptography
- miniz, for decompressing gzip data sent by Telegram
- qrcodegen, for making QR codes which can be used to log in
- libwebp, for converting custom emojis and stickers
- libjpeg, for converting images
- forumoji emojis

## Building
Telegacy is built with Microsoft Visual C++ 6.0 SP6, powered by Microsoft Platform SDK July 2000.

## Screenshots
<img width="1024" height="768" alt="xp" src="https://github.com/user-attachments/assets/4d5139c2-57b4-4ad0-80a0-4d38b4662ab1" />
<br>
<img width="1024" height="768" alt="me" src="https://github.com/user-attachments/assets/b1fc1345-cc93-4da7-b9aa-024d9a5c8d12" />
<br>
<img width="1024" height="768" alt="95" src="https://github.com/user-attachments/assets/daaf96ed-dc59-49f1-a14a-5667617b5ec2" />
<br>
<img width="1024" height="768" alt="nt351" src="https://github.com/user-attachments/assets/c39147a8-4be0-4874-adff-8d5b7c1240b0" />
