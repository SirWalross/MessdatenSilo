# DMS Messungen vom Silo

## Daten
- Die Daten der Messungen bis zum Vortag können unter [Veröffentlichungen/Releases](https://gitlab.cvh-server.de/Lennard/messdatensilo/-/releases/latest) heruntergeladen werden.
- Die Daten werden als `.mat` Datein gespeichert und sind jeweils von einer Woche.

## Funktionsweise
- Ein Cron Job führt nach jeden reboot and jeden Tag um 0:00 das `scripts/run.bash` skript aus.
- `run.bash` führt dann `python3 main.py` aus, welches die Daten für einen Tag sammelt, und läd diese dann im Anschluss auf gitlab.cvh-server.de hoch.
- Das `main.py` Programm liest immer die Daten von den Arduinos, mittelt diese über einen gewissen Zeitraum und speichert diese dann in `data/data` ab.
- `main.py` stoppt dann kurz vor Mitternacht, benennt `data/data` dann in `log.Jahr-Monat-Tag_Stunde.log` um und löscht die älteste Datei, falls zu viele Datein vorhanden sind.
- Der Zeitraum, die Anzahl zu behaltene Datein und weitere Parameter von dem Programm können in `config.yml` verändert werden.
- Zusätzlich werden noch weitere Log Datein geführt:
    - In `logs/*` werden logs vom `python3 main.py` geschrieben.
    - In `bash.log` werden logs von `run.bash` geschrieben.