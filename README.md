# Bikesla
The final project of Embedded-System Lab, NTUEE

# set up
1. Import Program from http://os.mbed.com/teams/mbed-os-examples/code/mbed-os-example-ble-Button/
2. modified codes in pretty-printer.h
Gap::Address_t ---> ble::address_t  
Gap::AddressType_t ---> ble::own_address_type_t  
&addr_type ---> addr_type  
GAP::Phy_t ---> ble::phy_t  
3. rm -r shields/TARGET_CORDIO_BLENRG
4. modified mbed_app.json, line 31
"USER_BUTTON" ---> "BUTTON1"  
5. 