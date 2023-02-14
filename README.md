# Load-Balancer-Consistent-Hashing
Simulated server load balancer using consistent hashing (hashring)

/* Copyright 2021 Calugaritoiu Ion-Victor */

	main.c --- parsarea comenzilor si apelarea functiilor potrivite

	- main: deschide fisierul de input si apeleaza functia "apply_requests";
	
	- get_key & get_key_value: obtin perechile cheie-valoare;
	
	- apply_requests: se primesc urmatoarele comenzi: store, retrieve, 
add_server si remove_server, acestea fiind implementate in fisierul 
"load balancer";

	load_balancer.c --- implementarea comenzilor si declarararea si
alocarea memoriei pentru structura load_balancer; aceasta  contine o lista 
inlantuita "server_list" ce va stoca serverele in ordine crescatoare a 
hash-urilor lor;
	
	- loader_store: apeleaza functia "find_hashring_spot" pentru a obtine
server-ul pe care trebuie stocata noua pereche cheie-valoare; se apeleaza
functia "server_store" pentru a insera aceste date pe server-ul gasit;

	- loader_retrieve: apeleaza functia "find_hashring_spot" pentru a 
obtine server-ul de pe care trebuie returnata valoarea cheii primite ca
parametru; returneaza valoarea intoarsa de catre functia server_retrieve;
		
		* find_hashring_spot: calculeaza hash-ul cheii primite ca
parametru; in functie de acest hash, se gaseste server-ul potrivit stocarii,
parcurgand server_list si comparand hash-urile servere-lor cu cel al cheii;
returneaza server-ul potrivit si actualizeaza parametrul "server_id";

	- loader_add_server: se calculeaza hash-ul in functie de id, folosind
functia hash_function_servers; se parcurge server-list comparand hash-urile 
pentru a gasi pozitia pe care noul nod trebuie adaugat; se repeta aceste 
operatii pentru a adauga si cele 2 replici are server-ului; la adaugarea 
ultimei replici, se verifica daca plasamentul cheilor corespunde ordinii 
crescatoare in hashring, folosind functia check_key_placement;

		* check_key_placement: apeleaza functia reassign_keys pentru 
fiecare server din lista; dupa fiecare apelare se stocheaza id-ul server-ului 
verificat intr-un hashtable "reassigned"(la fiecare iteratie se verifica daca
cheile nodului curent au fost deja "reassigned" folosind acest dictionar);

		* reassign_keys: gaseste toate cheile stocate in server-ul
primit ca parametru si apeleaza functia "loader_store" pentru a le recalcula
pozitia si pentru a le insera in server-ul potrivit.

	- loader_remove_server: se parcurge server-list pentru a obtine cele 3 
noduri care trebuiesc eliminate in urma stergerii unui server; atunci cand se 
obtine ultima replica a server-ului ce trebuie sters, se apeleaza functia 
"reassign_keys" pentru a distribui cheile care apartineau acestuia si se 
elibereaza memoria hashtable-ului si a nodului;
	
	- free_load_balancer: se parcurge server_list si se foloseste un 
hashtable "free_tables" utilizat pentru a memora care servere au avut hashtable
-ul deja eliberat; pentru fiecare nod neeliberat se apeleaza functia 
"free_server_memory"; dupa ce toate nodurile sunt eliberate, se elibereaza
memoria listei de servere si a structurii de load_balancer;

	server.c --- aloca memorie pentru o noua structura de tip server si 
apeleaza functiile corespunzatoare din "hashtable.c"
	
	- server_store: ht_put in server-ul primit ca paramtru;
	- server_remove: sterge o pereche cheie valoare(ht_remove_entry);
	- server_retireve: returneaza valoarea asociata cheii, sau NULL in 
cazul in care nu exista in hashtable(ht_get);
	- free_server_memory: elibereaza memoria folosita de hashtable(ht_free);

