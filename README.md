# Progetto di Laboratorio II

## Questa è la mia personale implementazione di un sistema bibliotecario distribuito.

Il progetto è diviso in 2 parti principali: 

- Server: è un sistema multitreaded in grado di gestire più connessioni contemporaneamente.
- Client: è un programma single threaded che interroga il server in base a determinati input

Sono presenti anche quattro librerie così divise:
- book.c: contiene tutte le funzioni utili al corretto utilizzo della struttura dati "single_book".
- linkedList.c: è la mia personale implementazione di linked list thread safe
- queue.c: è un'impementazione di coda thread safe fornita dalla professoressa
- util.c: contiene tutte le funzioni di utility

## Usage

Il progetto offre la possibilità di avviare i server ed i client singolarmente oppure avviare una sessione di test.

Per entrambe le modalità di utilizzo si deve compiere un'azione preliminare:

```bash
~/project_dir$ make prepfile
```

Successivamente: 
- per avviare singolarmente server e client:
    
    (verrà avviato un server, poi, dopo un'attesa di 1 secondo verrà avviato un client)

    -   ```bash
        ~/project_dir$ make runsingle
        ```
- per avviare la sessione di test: 

    -   ```bash
        ~/project_dir$ make test
        ``` 

I server terminano in automatico ma, se non si dovessero fermare, si può terminare con un SIGKILL con il comando: 
```bash
~/project_dir$ make hardstop
```
    
        



