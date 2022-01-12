# Bikesla
The final project of Embedded-System Lab, NTUEE

# set up
1. Import Program from http://os.mbed.com/teams/mbed-os-examples/code/mbed-os-example-ble-Button/  
2. Update Libraries, mbed-os.6.15.1  
3. modified codes in pretty-printer.h  
Gap::Address_t ---> ble::address_t  
Gap::AddressType_t ---> ble::own_address_type_t  
&addr_type ---> addr_type  
GAP::Phy_t ---> ble::phy_t  
4. rm -r shields img  
5. copy source/* mbed_app.json  
6. change mini_printf to std if needs.
