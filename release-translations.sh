#! /bin/sh
# Helper for Qt5 libs to generate all Octopi translations

# First we get all translations from Transifex
tx pull

# Then we release each of them
TRANSLATIONS="./resources/translations/*"

for f in $TRANSLATIONS
do
  lrelease-qt5 "$f"
done

# Repeat for Cachecleaner
cd cachecleaner || exit
tx pull

# And release each of them
for f in $TRANSLATIONS
do
  lrelease-qt5 "$f"
done

# Repeat for Repoeditor
cd ../repoeditor || exit
tx pull

# And release each of them
for f in $TRANSLATIONS
do
  lrelease-qt5 "$f"
done

