# CppTestTBot

The testing system via a Telegram bot.
Career guidance tests are implemented, it is possible to expand with other types of tests.

## Dependencies

```  
sudo apt-get install g++ make binutils cmake libssl-dev libboost-system-dev zlib1g-dev subversion doxygen libcurl4-openssl-dev
```

## Installation
```
git clone https://github.com/LegendLex/CppTestTBot
cd CppTestTBot/
git clone https://github.com/reo7sp/tgbot-cpp  
cmake .
cmake --build . --target install
```
## Start
```  
cd build/bin/
./profbot
```
