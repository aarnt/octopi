<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="el">
<context>
    <name>PasswordDialog</name>
    <message>
        <location filename="../passworddialog.ui" line="6"/>
        <source>LXQt sudo</source>
        <translation>LXQt sudo</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="42"/>
        <source>Copy command to clipboard</source>
        <translation>Αντιγραφή της εντολής στο πρόχειρο</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="45"/>
        <source>&amp;Copy</source>
        <translation>&amp;Αντιγραφή</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="83"/>
        <source>The requested action needs administrative privileges.&lt;br&gt;Please enter your password.</source>
        <translation>Η αιτηθείσα ενέργεια απαιτεί προνόμια διαχειριστή.&lt;br&gt;Παρακαλώ εισαγάγετε τον κωδικό πρόσβασης.</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="106"/>
        <source>LXQt sudo backend</source>
        <translation>Σύστημα υποστήριξης sudo LXQt</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="109"/>
        <source>A program LXQt sudo calls in background to elevate priveledges.</source>
        <translation>Ένα πρόγραμμα το οποίο καλεί το LXQt sudo για την παραχώρηση προνομίων</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="119"/>
        <source>Command:</source>
        <translation>Εντολή:</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="126"/>
        <source>Password:</source>
        <translation>Κωδικός πρόσβασης:</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="133"/>
        <source>Enter password</source>
        <translation>Εισαγάγετε τον κωδικό πρόσβασης</translation>
    </message>
    <message>
        <location filename="../passworddialog.cpp" line="60"/>
        <source>Attempt #%1</source>
        <translation>Προσπάθεια #%1</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../sudo.cpp" line="67"/>
        <source>Usage: %1 option [command [arguments...]]

GUI frontend for %2/%3

Arguments:
  option:
    -h|--help      Print this help.
    -v|--version   Print version information.
    -s|--su        Use %3(1) as backend.
    -d|--sudo      Use %2(8) as backend.
  command          Command to run.
  arguments        Optional arguments for command.

</source>
        <translation>Χρήση: %1 επιλογή [εντολή ορίσματα...]]

περιβάλλον συστήματος υποστήριξης για το %2/%3

Ορίσματα:
  επιλογή:
    -h|--help      Εκτύπωση της βοήθειας.
    -v|--version   Εκτύπωση της έκδοσης.
    -s|--su        Χρήση του %3(1) ως σύστημα υποστήριξης.
    -d|--sudo      Χρήση του %2(8) ως σύστημα υποστήριξης.
  εντολή          Εντολή προς εκτέλεση.
  ορίσματα        Προαιρετικά ορίσματα της εντολής.

</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="84"/>
        <source>%1 version %2
</source>
        <translation>%1 έκδοση %2
</translation>
    </message>
</context>
<context>
    <name>Sudo</name>
    <message>
        <location filename="../sudo.cpp" line="183"/>
        <source>%1: no command to run provided!</source>
        <translation>%1: δεν παρείχατε κάποια εντολή προς εκτέλεση!</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="190"/>
        <source>%1: no backend chosen!</source>
        <translation>%1: δεν έχετε επιλέξει το σύστημα υποστήριξης!</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="207"/>
        <source>Syscall error, failed to fork: %1</source>
        <translation>Σφάλμα Syscall, αποτυχία δικράνωσης: %1</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="234"/>
        <source>unset</source>
        <extracomment>shouldn&apos;t be actually used but keep as short as possible in translations just in case.</extracomment>
        <translation>ανενεργό</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="270"/>
        <source>%1: Detected attempt to inject privileged command via LC_ALL env(%2). Exiting!
</source>
        <translation>%1: Εντοπίστηκε απόπειρα έγχυσης προνομιούχας εντολής μέσω του LC_ALL env(%2). Εγκατάλειψη!</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="314"/>
        <source>Syscall error, failed to bring pty to non-block mode: %1</source>
        <translation>Σφάλμα Syscall, αποτυχία διάθεσης του pty σε ανεμπόδιστη λειτουργία: %1</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="322"/>
        <source>Syscall error, failed to fdopen pty: %1</source>
        <translation>Σφάλμα Syscall, αποτυχία του fdopen pty: %1</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="291"/>
        <source>%1: Failed to exec &apos;%2&apos;: %3
</source>
        <translation>%1: Αποτυχία εκτέλεσης του «%2»: «%3»
</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="353"/>
        <source>Child &apos;%1&apos; process failed!
%2</source>
        <translation>Η θυγατρική διεργασία «%1» απέτυχε!
%2</translation>
    </message>
</context>
</TS>
