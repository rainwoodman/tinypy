BACKEND=tpvm
XFAIL=0
DEBUG=0

# Call getopt to validate the provided input. 
OPT=$(getopt -ogxB: -l debug,xfail,backend: -- "$@")

if [[ $? -ne 0 ]] ; then
    echo "Incorrect options provided"
    exit 1
fi

eval set -- "$OPT"

while true; do
    case "$1" in
    -x | --xfail)
        XFAIL=1
        ;;
    -g | --debug)
        DEBUG=1
        ;;
    -B | --backend)
        shift; # The arg is next in position args
        BACKEND=$1
        if [[ ! ${BACKEND} =~ tpy|tpvm ]]; then
            echo "Incorrect options provided. Use tpy or tpvm"
            exit 1
        fi
        ;;
    --)
        shift
        break
        ;;
    esac
    shift
done

shift $((OPTIND -1))

TESTS=$@
echo $TESTS

if [[ "${DEBUG}" == "1" ]]; then
    TPVM=./tpvm-dbg
    TPY=./tpy
    TPC=./tpc
else
    TPVM=./tpvm
    TPY=./tpy
    TPC=./tpc
fi

function run {
    tpc=${1//.py/.tpc}
    if [[ "${tpc}" == "$1" ]]; then
        echo $1 does not end with .py
        exit 1
    fi
    if [[ "${BACKEND}" == "tpvm" ]]; then
        echo "${TPC} -o ${tpc} $1"
        echo "${TPVM} ${tpc}"
        "${TPC}" -o ${tpc} $1 || return 1
        "${TPVM}" ${tpc} || return 1
    else
        echo "${TPY} $1"
        "${TPY}" $1 || return 1
    fi
}

st=0
for i in ${TESTS[@]}; do
    if ! run "${i}"; then
        echo ==== ${st}
        if [[ ${XFAIL} -ne 0 ]]; then
            exit 255
        fi
        st=1
    fi
done

if [ "${st}" -ne 0 ]; then
    echo Some tests failed.
    exit 255
else
    echo All tests passed.
fi
