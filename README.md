# my-wget


This is a simple implementation of the well known wget. It can be used to download a html page.
Usage: ./myclient [-r] [-e] [-o <log_file>] http://<server_name>/<link_page>
-r : Download all the html link in the link_page up to 5 levels recursivly
-e : Download every relativ link in page
-o : It creates a log file were is written all the errors and all the succsesfull requests.
