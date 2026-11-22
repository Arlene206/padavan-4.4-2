#!/bin/sh

if [ ! -x /usr/bin/openssl ] && [ ! -x /opt/bin/openssl ] ; then
  echo "Unable to find the 'openssl' executable!"
  echo "Please install 'openssl-util' package from Entware."
  exit 1
fi

umask 0077

## path to openvpn binary
OPENVPN=/usr/sbin/openvpn
## CA cert valid, days
CA_DAYS=3653
## default value for certs valid period, days
CERT_DAYS=365
## cert and key file names
CA_CRT=ca.crt
CA_KEY=ca.key
SERVER_KEY=server.key
SERVER_CRT=server.crt
CLIENT_KEY=client.key
CLIENT_CRT=client.crt
TA_KEY=ta.key
STC2_KEY=stc2.key
ECPARAM=ecparam.pem
## number of bits to use when generate new key
RSA_BITS=1024
## number of bits for prime
DH_BITS=1024
## CA common name
CA_CN="OpenVPN CA"
## temp config file with cert extensions
SSL_EXT_FILE=/etc/ssl/openssl_ext1.cnf
## certs storage path
CRT_PATH=/etc/storage/openvpn
CRT_PATH_SRV="$CRT_PATH/server"
if [ -z "$CRT_PATH_CLI" ] ; then
  CRT_PATH_CLI="$CRT_PATH/client"
fi

# Check if -sha256 is supported
DGST_ALG="-sha1"
openssl list -1 --digest-commands 2>&1 | grep -q 'sha256' && DGST_ALG="-sha256"

func_help() {
  local BOLD="echo -ne \\033[1m"
  local NORM="echo -ne \\033[0m"
  echo >&2
  echo "Create certificates for OpenVPN client/server. For more info see:" >&2
  echo "http://openvpn.net/index.php/open-source/documentation/howto.html" >&2
  echo "https://code.google.com/p/rt-n56u/wiki/HowToConfigureOpenvpnServer" >&2
  echo >&2
  echo "`$BOLD`Usage:`$NORM` $0 command [ args ]" >&2
  echo >&2
  echo "    `$BOLD`commands:`$NORM` [ server, client, client_csr, client_sign ]" >&2
  echo >&2
  echo "    `$BOLD`server`$NORM` [ -n `$BOLD`common_name`$NORM` ] [ -b `$BOLD`rsa_bits/ec_name`$NORM` ] [ -d `$BOLD`days_valid`$NORM` ]" >&2
  echo "           The following files for OpenVPN server are created:" >&2
  echo "           - root CA key and certificate" >&2
  echo "           - server key and certificate" >&2
  echo "           - Diffie-Hellman parameters key" >&2
  echo "           - TLS-Auth HMAC signature key" >&2
  echo "           `$BOLD`Note:`$NORM` $CA_CRT and ${TA_KEY}(if TLS-Auth or TLS-Crypt is used) should be sent to clients." >&2
  echo >&2
  echo "    `$BOLD`client`$NORM` -n `$BOLD`common_name`$NORM` [ -b `$BOLD`rsa_bits/ec_name`$NORM` ] [ -d `$BOLD`days_valid`$NORM` ]" >&2
  echo "           Create both client key and sign it on server side. It is not quite correct," >&2
  echo "           but it saves time if you administer both server and client devices." >&2
  echo >&2
  echo "    `$BOLD`client_csr`$NORM` -n `$BOLD`common_name`$NORM` [ -b `$BOLD`rsa_bits/ec_name`$NORM` ]" >&2
  echo "           The following files for OpenVPN client are created:" >&2
  echo "           - client key" >&2
  echo "           - certificate signing request (client.csr)" >&2
  echo "           `$BOLD`Note:`$NORM` This request should be signed with OpenVPN server CA certificate." >&2
  echo >&2
  echo "    `$BOLD`client_sign`$NORM` -f `$BOLD`csr_file_path`$NORM` [ -d `$BOLD`days_valid`$NORM` ]" >&2
  echo "           Create client certificate." >&2
  echo >&2
  echo "    `$BOLD`ssl_view`$NORM` -f `$BOLD`crt/csr_file_path`$NORM`" >&2
  echo "           Allows you to see the contents of the requests or certificates using the" >&2
  echo "           `$BOLD`openssl`$NORM` utility." >&2
  echo >&2
  echo >&2
  echo "`$BOLD`Example:`$NORM`" >&2
  echo "  If you are new to OpenVPN but want to connect server and client," >&2
  echo "  you can create certificates using:" >&2
  echo "    `$BOLD`$0 server`$NORM`" >&2
  echo "    `$BOLD`$0 client -n client1`$NORM`" >&2
  echo "  Then copy the following files to client:" >&2
  echo "    `$BOLD`$CA_CRT`$NORM`, `$BOLD`$TA_KEY`$NORM` from `$BOLD`$CRT_PATH_SRV`$NORM`" >&2
  echo "    `$BOLD`$CLIENT_KEY`$NORM`, `$BOLD`$CLIENT_CRT`$NORM` from `$BOLD`$CRT_PATH_CLI`$NORM`" >&2
  echo >&2
  exit 1
}

ACTION=$1
shift
case "$ACTION" in
  client) ;;
  client_csr) ;;
  client_sign) ;;
  server) ;;
  ssl_view) ;;
  *) func_help ;;
esac

[ $(( $# % 2 )) -ne 0 ] && func_help

while [ $# -gt 0 ] ; do
  case "$1" in
    -b) RSA_BITS=$2   ; shift 2 ;;
    -d) CERT_DAYS=$2  ; shift 2 ;;
    -n) CN=$2         ; shift 2 ;;
    -f) CSR_PATH=$2   ; shift 2 ;;
    -s) CRL_SERIAL=$2 ; shift 2 ;;
     *) func_help     ;;
  esac
done


write_ext_cfs() {
  rm -f $SSL_EXT_FILE
  cat > $SSL_EXT_FILE << EOF
[ server ]
extendedKeyUsage=serverAuth
keyUsage=critical,digitalSignature,keyEncipherment
[ client ]
extendedKeyUsage=clientAuth
keyUsage=critical,digitalSignature
EOF
}

echo_done() {
  echo -e " *\033[60G [ done ]"
}

echo_process() {
  echo -ne " + $@\r"
}

make_cert() {
  local CA_TRUE="" key_status req_status
  [ "${7:-}" = "ca" ] && CA_TRUE="-x509"
  if [ ! -s "$1" ]; then
    echo_process "Creating private key: $1"
    case "$4" in
      [bpsw]*)
        if openssl ecparam -name "$4" -genkey -out "$1"; then :; else
          key_status=$?; rm -f "$1"; echo "Error: private key generation failed for $1 (status=$key_status)" >&2; return "$key_status"
        fi ;;
      ed*)
        if openssl genpkey -algorithm "$4" -out "$1"; then :; else
          key_status=$?; rm -f "$1"; echo "Error: private key generation failed for $1 (status=$key_status)" >&2; return "$key_status"
        fi ;;
      [1-9]*)
        if openssl genrsa -out "$1" "$4"; then :; else
          key_status=$?; rm -f "$1"; echo "Error: private key generation failed for $1 (status=$key_status)" >&2; return "$key_status"
        fi ;;
      *) echo "Error: unsupported private key type: $4" >&2; return 1 ;;
    esac
    if [ ! -s "$1" ]; then
      rm -f "$1"
      echo "Error: private key generator produced no key for $1" >&2
      return 1
    fi
  fi
  if [ ! -f "$1" ] || [ ! -s "$1" ]; then
    echo "Error: private key is missing or empty: $1" >&2
    return 1
  fi
  if chmod 600 "$1"; then :; else
    key_status=$?; echo "Error: cannot protect private key $1 (status=$key_status)" >&2; return "$key_status"
  fi
  echo_process "Creating $2: $5"
  if openssl req -nodes $CA_TRUE -days "$3" -new -outform PEM -out "$2" -key "$1" "$6" -subj "/CN=$5" &>/dev/null; then
    :
  else
    req_status=$?; rm -f "$2"; echo "Error: certificate request generation failed for $2 (status=$req_status)" >&2; return "$req_status"
  fi
  if [ ! -s "$2" ]; then
    rm -f "$2"
    echo "Error: certificate request generator produced no output: $2" >&2
    return 1
  fi
  echo_done
  return 0
}

sign_cert() {
  local sign_status cleanup_status
  if [ ! -s "$1" ] || [ ! -s "$2" ]; then
    echo "Error: CA key or certificate not found" >&2
    return 1
  fi
  if [ ! -s "$3" ]; then
    echo "Error: certificate signing request not found: $3" >&2
    return 1
  fi
  if write_ext_cfs; then :; else
    sign_status=$?; echo "Error: cannot prepare certificate extensions (status=$sign_status)" >&2; return "$sign_status"
  fi
  echo_process "Signing $4"
  if openssl x509 -req -in "$3" -CA "$2" -CAkey "$1" -CAcreateserial -clrext -out "$4" "-$6" -extfile "$SSL_EXT_FILE" -days "$5" -extensions "$7" &>/dev/null; then
    :
  else
    sign_status=$?; rm -f "$4"; echo "Error: certificate signing failed for $4 (status=$sign_status)" >&2; return "$sign_status"
  fi
  if [ ! -s "$4" ]; then
    rm -f "$4"
    echo "Error: certificate signer produced no output: $4" >&2
    return 1
  fi
  if rm -f "$3" && rm -f ca.srl; then :; else
    cleanup_status=$?; echo "Error: certificate generated but signing-request cleanup failed (status=$cleanup_status)" >&2; return "$cleanup_status"
  fi
  echo_done
  return 0
}

make_dh() {
  local dh_status
  if [ -s dh1024.pem ]; then
    echo_process "Skipping DH Parameters. File exists"
  else
    echo_process "Creating DH Parameters (may take long time, be patient)"
    if openssl dhparam -out dh1024.pem "$1" &>/dev/null; then :; else
      dh_status=$?; rm -f dh1024.pem; echo "Error: DH parameter generation failed (status=$dh_status)" >&2; return "$dh_status"
    fi
    if [ ! -s dh1024.pem ]; then
      rm -f dh1024.pem
      echo "Error: DH parameter generator produced no output" >&2
      return 1
    fi
  fi
  echo_done
  return 0
}

make_ta_tc2() {
  #
  # $1 --> ta key name
  #
  # $2 --> server tc2 key name
  #

  if [ ! -x "$OPENVPN" ] ; then
    echo_process "Skipping TLS Auth/Crypt key. $OPENVPN not found."
    echo_done
    return 1
  fi

  if [ -s "$1" ] ; then
    echo_process "Skipping TLS Auth key. File exists"
  else
    echo_process "Creating TLS Auth key"
    if "$OPENVPN" --genkey secret "$1" &>/dev/null; then
      :
    else
      key_status=$?
      rm -f "$1"
      echo "Error: TLS Auth key generation failed (status=$key_status)" >&2
      return "$key_status"
    fi
    if [ ! -s "$1" ] ; then
      rm -f "$1"
      echo "Error: TLS Auth key generator produced no key" >&2
      return 1
    fi
  fi
  if chmod 600 "$1"; then
    :
  else
    key_status=$?
    rm -f "$1"
    echo "Error: cannot protect TLS Auth key (status=$key_status)" >&2
    return "$key_status"
  fi
  echo_done

  if [ -s "$2" ] ; then
    echo_process "Skipping TLS Crypt v2 server key. File exists"
  else
    echo_process "Creating TLS Crypt v2 server key"
    if "$OPENVPN" --genkey tls-crypt-v2-server "$2" &>/dev/null; then
      :
    else
      key_status=$?
      rm -f "$2"
      echo "Error: TLS Crypt v2 server key generation failed (status=$key_status)" >&2
      return "$key_status"
    fi
    if [ ! -s "$2" ] ; then
      rm -f "$2"
      echo "Error: TLS Crypt v2 server key generator produced no key" >&2
      return 1
    fi
  fi
  if chmod 600 "$2"; then
    :
  else
    key_status=$?
    rm -f "$2"
    echo "Error: cannot protect TLS Crypt v2 server key (status=$key_status)" >&2
    return "$key_status"
  fi
  echo_done
}

server() {
  local CRT_PATH_X=$CRT_PATH_SRV setup_status
  if [ ! -d "$CRT_PATH_X" ]; then
    if mkdir -m 755 -p "$CRT_PATH_X"; then :; else
      setup_status=$?; echo "Error: cannot create certificate directory $CRT_PATH_X (status=$setup_status)" >&2; return "$setup_status"
    fi
  fi
  if ! cd "$CRT_PATH_X"; then echo "Error: can't cd to $CRT_PATH_X" >&2; return 1; fi
  make_cert "$CA_KEY" "$CA_CRT" "$CA_DAYS" "$RSA_BITS" "$CA_CN" "$DGST_ALG" ca || return $?
  [ -z "$CN" ] && CN="OpenVPN Server"
  make_cert "$SERVER_KEY" server.csr "$CERT_DAYS" "$RSA_BITS" "$CN" "$DGST_ALG" || return $?
  sign_cert "$CA_KEY" "$CA_CRT" server.csr "$SERVER_CRT" "$CERT_DAYS" "$DGST_ALG" server || return $?
  make_dh "$DH_BITS" || return $?
  make_ta_tc2 "$TA_KEY" "$STC2_KEY"
}

client_csr() {
  local CRT_PATH_X=$CRT_PATH_CLI setup_status
  if [ ! -d "$CRT_PATH_X" ]; then
    if mkdir -m 755 -p "$CRT_PATH_X"; then :; else
      setup_status=$?; echo "Error: cannot create client certificate directory $CRT_PATH_X (status=$setup_status)" >&2; return "$setup_status"
    fi
  fi
  if ! cd "$CRT_PATH_X"; then echo "Error: can't cd to $CRT_PATH_X" >&2; return 1; fi
  [ -z "$CN" ] && func_help
  make_cert "$CLIENT_KEY" client.csr "$CERT_DAYS" "$RSA_BITS" "$CN" "$DGST_ALG"
}

client_sign() {
  [ -z "$CSR_PATH" ] && func_help
  if [ "${CSR_PATH:0:1}" != "/" ]; then CSR_PATH=`pwd`/"$CSR_PATH"; fi
  local CRT_PATH_X=$CRT_PATH_SRV setup_status
  if [ ! -d "$CRT_PATH_X" ]; then
    if mkdir -m 755 -p "$CRT_PATH_X"; then :; else
      setup_status=$?; echo "Error: cannot create server certificate directory $CRT_PATH_X (status=$setup_status)" >&2; return "$setup_status"
    fi
  fi
  if ! cd "$CRT_PATH_X"; then echo "Error: can't cd to $CRT_PATH_X" >&2; return 1; fi
  if [ ! -f "$CSR_PATH" ]; then echo "Error: $CSR_PATH - file not found" >&2; return 1; fi
  sign_cert "$CA_KEY" "$CA_CRT" "$CSR_PATH" "${CSR_PATH%.*}.crt" "$CERT_DAYS" "$DGST_ALG" client
}

client() {
  client_csr || return $?
  CSR_PATH="$CRT_PATH_CLI/client.csr"
  client_sign
}

ssl_view () {
  [ -z "$CSR_PATH" ] && func_help
case ${CSR_PATH##*.} in
	crt) openssl x509 -in "$CSR_PATH" -noout -text ;;
	csr) openssl req -in "$CSR_PATH" -noout -text ;;
	*)   func_help ;;
esac
}

eval $ACTION
