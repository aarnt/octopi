<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fr">
<context>
    <name>PasswordDialog</name>
    <message>
        <location filename="../passworddialog.ui" line="6"/>
        <source>LXQt sudo</source>
        <translation>LXQt sudo (superutilisateur)</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="42"/>
        <source>Copy command to clipboard</source>
        <translation>Copier la commande dans le presse-papier</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="45"/>
        <source>&amp;Copy</source>
        <translation>&amp;Copier</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="83"/>
        <source>The requested action needs administrative privileges.&lt;br&gt;Please enter your password.</source>
        <translation>L&apos;action demandée nécessite des privilèges d&apos;administrateur.&lt;br&gt;Veuillez entrer votre mot de passe.</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="106"/>
        <source>LXQt sudo backend</source>
        <translation>LXQt sudo en arrière-plan</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="109"/>
        <source>A program LXQt sudo calls in background to elevate privileges.</source>
        <translation>Un programme que LXQt sudo appelle en arrière-plan pour faire une élévation de privilèges.</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="119"/>
        <source>Command:</source>
        <translation>Commande :</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="126"/>
        <source>Password:</source>
        <translation>Mot de passe :</translation>
    </message>
    <message>
        <location filename="../passworddialog.ui" line="133"/>
        <source>Enter password</source>
        <translation>Entrer le mot de passe</translation>
    </message>
    <message>
        <location filename="../passworddialog.cpp" line="60"/>
        <source>Attempt #%1</source>
        <translation>Essai #%1</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../sudo.cpp" line="75"/>
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
        <translation>Usage&#x202f;: %1 option [commande [arguments...]]

Interface graphique pour %2/%3

Arguments&#x202f;:
  option&#x202f;:
    -h|--help      Affiche cette aide.
    -v|--version   Affiche la version.
    -s|--su        Utiliser %3(1) comme backend.
    -d|--sudo      Utiliser %2(8) comme backend.
  command        Commande à exécuter.
  arguments        Arguments optionnels pour la commande.

</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="92"/>
        <source>%1 version %2
</source>
        <translation>%1 version %2
</translation>
    </message>
</context>
<context>
    <name>Sudo</name>
    <message>
        <location filename="../sudo.cpp" line="195"/>
        <source>%1: no command to run provided!</source>
        <translation>%1&#x202f;: aucune commande n&apos;a été spécifiée&#x202f;!</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="202"/>
        <source>%1: no backend chosen!</source>
        <translation>%1&#x202f;: aucun Backend n&apos;a été choisi&#x202f;!</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="219"/>
        <source>Syscall error, failed to fork: %1</source>
        <translation>Erreur d&apos;appel système, échec de la conversion&#x202f;: %1</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="246"/>
        <source>unset</source>
        <extracomment>shouldn&apos;t be actually used but keep as short as possible in translations just in case.</extracomment>
        <translation>non spécifié</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="295"/>
        <source>%1: Detected attempt to inject privileged command via LC_ALL env(%2). Exiting!
</source>
        <translation>%1&#x202f;: Tentative détectée d&apos;injection d&apos;une commande privilégiée via LC_ALL env(%2). En sortant&#x202f;!
</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="337"/>
        <source>Syscall error, failed to bring pty to non-block mode: %1</source>
        <translation>Erreur d&apos;appel système, échec du transfert de pty en mode sans blocage&#x202f;: %1</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="345"/>
        <source>Syscall error, failed to fdopen pty: %1</source>
        <translation>Erreur d&apos;appel système, échec de fdopen pty&#xa0;: %1</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="314"/>
        <source>%1: Failed to exec &apos;%2&apos;: %3
</source>
        <translation>%1&#x202f;: Impossible d&apos;exécuter &apos;%2&apos;&#x202f;: %3
</translation>
    </message>
    <message>
        <location filename="../sudo.cpp" line="376"/>
        <source>Child &apos;%1&apos; process failed!
%2</source>
        <translation>Échec du processus fils &apos;%1&apos; !
%2</translation>
    </message>
</context>
</TS>
