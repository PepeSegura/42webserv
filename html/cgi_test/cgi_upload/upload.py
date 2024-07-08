#!/usr/bin/env python3


import cgi, os 
#Debugging
import cgitb; cgitb.enable()
import sys 
  
form = cgi.FieldStorage()
upload_dir = 'uploaded_files'

message = '';
if len(sys.argv):
    message = sys.argv
response_body = """
<html><body>
<p>Parametros recibidos: %s</p>
</body></html>
""" % (message,)
print(response_body)
