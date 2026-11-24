var ipmonitor_last = [<% get_static_client(); %>];
var nmap_fullscan_last = '' + <% nvram_get_js("", "networkmap_fullscan"); %> + '';