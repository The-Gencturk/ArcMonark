@echo off
REM ArcMonark'i nereden calistirilirsa calistirilsin proje klasorunden baslatir.
REM %~dp0 = bu .bat dosyasinin bulundugu klasor (proje koku).
pushd "%~dp0"
wsl make run
popd
