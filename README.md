## Table of contents
* [Project description](#project-description)
* [Technologies used](#technologies-used)

## Project description

A game of general knowledge programmed on ubuntu. </br>
Every player can register with an unique username for entering the current game. 
The users have to answer in a matter of seconds a set of questions in the order they registered 
(they can answer whenever they want in that period of time or they can skip the question).
They are given four options from which they are asked to choose one. </br>
Their answers are verified by the server and for every right answer that they gave, five points are added to their score. 
The program is not affected if one of the players leaves the game. 
After the final question, when everybody sent their answers, the server determines the winner(or the winners) and prints him on the screen of every player. </br>
A new round can start after that with new players. </br>

## Technologies used
It is a multithreading program that supports as many clients. 
It is based on the TCP/IP protocol and for storing the questions an SQLite3 database is created and filled.
