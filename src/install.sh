JSON_MODEL_PATH=/usr/local/share/projewski/jsonModeler

install -d $JSON_MODEL_PATH;
install -d /usr/local/include/projewski/jsonModeler;

install ./jsonModeler.sh /usr/local/bin/jsonModeler.sh
install ./jsonModel.c $JSON_MODEL_PATH/jsonModel.c
install ./jsonModel.h /usr/local/include/projewski/jsonModeler/jsonModel.h
