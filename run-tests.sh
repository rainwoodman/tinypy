BACKEND=tpvm
XFAIL=0
DEBUG=0

# Call getopt to validate the provided input. 
OPT=$(getopt -oxB: -l xfail,backend: -- "$@")

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
    -B | --backend)
        shift; # The arg is next in position args
        BACKEND=$1
        if [[ ! ${BACKEND} =~ tpy|tpvm|tpvm-dbg|tpy-shared ]]; then
            echo "Incorrect options provided. Use tpy, tpvm, tpvm-dbg, or tpy-shared"
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

TPC=./tpc

function run {
    tpc=${1//.py/.tpc}
    if [[ "${tpc}" == "$1" ]]; then
        echo $1 does not end with .py
        exit 1
    fi
    if [[ "${BACKEND}" =~ tpvm|tpvm-dbg ]]; then
        echo "${TPC} -o ${tpc} $1"
        echo "./${BACKEND} ${tpc}"
        "${TPC}" -o ${tpc} $1 || return 1
        "./${BACKEND}" ${tpc} || return 1
    else
        echo "./${BACKEND} $1"
        "./${BACKEND}" $1 || return 1
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
