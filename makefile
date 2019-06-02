ftclient:
	dos2unix ftclient.py
	chmod +x ftclient.py

ftserver:

	gcc ftserver.c ftutilities.c -o ftserver.exe