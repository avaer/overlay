# build
echo building...

$ProgressPreference = 'SilentlyContinue'

Invoke-WebRequest "https://github.com/Hibbiki/chromium-win64/releases/download/v79.0.3945.130-r706915/chrome.sync.7z" -OutFile "chrome.7z"
7z x chrome.7z
rm chrome.7z

ls
cd device\vr
mkdir build
cd build
cp ..\build.ps1 .
& '.\build.ps1'
cd ..\..\..

echo zipping artifact... 
7z a xrchrome.zip -r .\device\vr\build\mock_vr_clients\bin\
echo done zipping artifact

ls

echo done