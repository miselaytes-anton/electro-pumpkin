./scripts/build_project.sh ./projects/pumpkin/ --force -m 'CPPFLAGS=-std=c++11'

./scripts/halt_board.sh 

./scripts/set_startup.sh startup ./projects/pumpkin/

./scripts/set_startup.sh nostartup ./projects/pumpkin/
