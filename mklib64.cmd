setlocal
cls
@call config.cmd
copy %KX_KDB_PATH%\w64\q.lib .\q64.lib
echo LIBRARY q.exe >q64.def
echo EXPORTS>>q64.def
bash -c "nm -p q64.lib|egrep 'T [^.]'|sed 's/0* T //'" >>q64.def
dlltool -v -l libq64.a -d q64.def
