#!/bin/sh

# This script creates a new snapshot of the system using TimeShift and removes the last one (if there are only 2 available)
# It must be located in /usr/lib/octopi for Octopi and Notifier to call it before a System Upgrade

echo -e "Creating a restore point...\n"

# Create a new snap
timeshift --create

nsnaps=`timeshift --list-snapshots | grep ">" -c`

if [ $nsnaps -eq 2 ]
then
  echo "Retrieving the name of older snapshot..."
  # Get the name of the older snap
  oldersnap=`timeshift --list-snapshots | grep ">" | grep "^0" | awk '{ print $3 }'`

  # Remove older snap
  echo "Removing snapshot $oldersnap..."
  timeshift --delete --snapshot $oldersnap
fi

echo -e "\nRestore point has been created!\n"
