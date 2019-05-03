import os
pull1 = 'git pull origin master'
pull2 = 'git submodule update --init --recursive'
pull3 = 'git submodule foreach git pull origin master'
pull4 = 'git submodule foreach --recursive git submodule update --init'
os.system(pull1)
os.system(pull2)
os.system(pull3)
os.system(pull4)