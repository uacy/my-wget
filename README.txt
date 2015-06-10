Untesu Albert Cristian 324 CB


	Programul meu downloadeaza de la un server o pagina html. Se poate apela cu
mai multe optiuni: ./myclient [-r] [-e] [-o <fisier_log>] 
http://<nume_server>/<cale_catre_pagina>. Optiunile urmatoare semnifica:
-r inseamna recursiv, -e inseamna everything, iar -o reprezinta scrierea
erorilor intr-un fisier log. Pentru implementarea programului am folosit
limbajul de programare C. 
	Am folosit mai multe functi ajutatoare pentru a face codul cat mai usor de 
inteles. Functia messageeror() scrie o eroare cand nu sunt folosite argumentele
asa cum au fost intentionate sa fie folosite si inchide programul cu statusul 
-1. Funcita getcurent intoarce un string cu calea curenta. Functi checkargv() 
verifica fiecare argument trimis programului si atribuie flagurile pentru 
fiecare optiune care a fost apelata. Daca argumentul nu este valid intoarce un 
mesaj de eroare. Functile Readline() si send_command sunt luate din
laboratorul cu posta electronica si modificate ca sa se potriveasca in 
programul meu. removehtml() sterge numele pagini html dintr-o cale 
relativa si returneaza calea pana la pagina. parser() face parsarea unui
link (din <a href="link" /a> returneaza link). addlink() adauga un pagina
html pentru a fi procesata mai tarziu si se ocupa sa nu apara duplicate.
addfile() face acelasi lucru ca addlink dar este pentru orice fisier care 
nu este html. checklink() verifica daca intr-un string se afla un link
valid care trebuie sa fie parsat mai tarziu. Returneaza 1 daca este un link
valid in interiorul stringului, altfel 0. get_ip() se ocupa sa obtina adresa
ip de la un host. 
	getpage() este functia care face toata treaba. Parseaza pagina ceruta,
creaza conexiune la server si face cerere la server. Daca primeste raspuns ok
de la server incepe sa creeze structura de foldere ca pe server. Dupa care 
incepe scrierea in fisier. Daca se cere o pagina html se scrie text, iar daca 
este un alt fel de fisier se scrie binar. La paginile html in acelasi timp cand
le scrie in fisier verifica daca sunt linkuri valide care trebuie tratate si le
adauga in vectorul global care va fi tratat mai tarziu. Dupa ce o pagina a fost
terminata de downloadat se intoarce in folderul root (/) si se inchide
conexiunea la server. open_socket() se ocupa sa faca o conexiune la server si 
se foloseste de fiecare data cand se doreste downloadarea unei pagini.
	In main mai intai se verifica argumentele si se ruleaza programul in 
functie de parametri dati. Se parseaza pagina initiala din care se obtine 
hostul si calea de pe server a pagini dorite. Se creaza folder cu numele
serverului dupa care se face apelul pentru pagina data ca parametru.
Daca sunt dati parametri -r sau -e se apeleaza in continuare.

	Parametri pot fi dati in orice ordine cu singura conditie ca numele fisierului
log sa fie imediat dupa flagul "-o", altfel programul crapa.
