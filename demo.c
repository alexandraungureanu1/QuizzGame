#include <sqlite3.h>
#include <stdio.h>

int main(void)
{
    sqlite3 *db;
    char *err_msg = 0;

    int rc=sqlite3_open("database.db",&db);

    if(rc!=SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }

    char * sql="DROP TABLE IF EXISTS intrebari;"
               "CREATE TABLE intrebari(id INT, text_intrebare TEXT, raspuns_1 TEXT, raspuns_2 Text, raspuns_3 TEXT, raspuns_4 TEXT, raspuns_corect TEXT);"
               "INSERT INTO intrebari VALUES(1,'Cine este Barack Obama?', 'a. fost presedinte','b. un cantaret celebru', 'c. magician', 'd. doctor', 'a');"
               "INSERT INTO intrebari VALUES(2,'Care este capitala Braziliei?', 'a. Rio','b. Brasil', 'c. Paris', 'd. New York', 'b');"
               "INSERT INTO intrebari VALUES(3,'Care a fost primul om pe luna?', 'a. Iuri Gagarin','b. Neil Armstrong', 'c. Pete Conrad', 'd. Buzz Aldrin', 'b');"
               "INSERT INTO intrebari VALUES(4,'Care este cel mai lung fluviu din Europa?', 'a. Dunarea','b. Sena', 'c. Tamisa', 'd. Volga', 'd');"
               "INSERT INTO intrebari VALUES(5,'Care este cel mai estic oras al Romaniei?', 'a. Arad','b. Timisoara', 'c. Sulina', 'd. Sf. Gheorghe', 'c');"
               "INSERT INTO intrebari VALUES(6,'Cine a scris Traviata?', 'a. Giuseppe Verdi','b. Wolfgang Amadeus Mozart', 'c. Franz Liszt', 'd. Frederic Chopin', 'a');"
               "INSERT INTO intrebari VALUES(7,'Care este animalul desertului?', 'a. ghepardul','b. zebra', 'c. rinocerul', 'd. camila', 'd');"
               "INSERT INTO intrebari VALUES(8,'La ce data a inceput Primul Razboi Mondial?', 'a. 1 septembrie 1914','b. 14 martie 1915', 'c. 28 iulie 1914', 'd. 15 septembrie 1914', 'c');"
               "INSERT INTO intrebari VALUES(9,'Cine a realizat prima calatorie in jurul lumii?', 'a. Cristofor Columb','b. Ludovic al XVIII-lea', 'c. Marco Polo ', 'd. Fernando Magellan', 'd');";


    rc=sqlite3_exec(db, sql, 0,0,&err_msg);

    if(rc!=SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);
        
        return 1;
    }

    sqlite3_close(db);

    return 0;
}