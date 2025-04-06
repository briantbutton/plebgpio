#!/bin/sh

cat <<EOT | sudo tee -a debian/plebgpio/DEBIAN/postinst > /dev/null

# Automatically appended section
sudo chown root:dialout /bin/plebgpio
sudo chmod 4755 /bin/plebgpio
sudo setcap 'cap_fowner,cap_chown+ep' /bin/plebgpio
# Automatically appended section

EOT
