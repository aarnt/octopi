#! /bin/sh
# Helper for Qt5 libs to generate all Octopi translations

# First we get all translations from Transifex
tx pull -a

# Then we release each of them
TRANSLATIONS="./resources/translations/*"

for f in $TRANSLATIONS
do
  lrelease "$f"
done

# Repeat for Cachecleaner
cd cachecleaner || exit
tx pull -a

# And release each of them
for f in $TRANSLATIONS
do
  lrelease "$f"
done

# Repeat for Repoeditor
cd ../repoeditor || exit
tx pull -a

# And release each of them
for f in $TRANSLATIONS
do
  lrelease "$f"
done

