TPVM=./tpvm
XFAIL=0
if [[ "$1" == "-dbg" ]]; then
    TPVM=./tpvm-dbg
    shift
fi

if [[ "$1" == "-xfail" ]]; then
    XFAIL=1
    shift
fi

TESTS=$@

# `TPY=1 make test` to run with tpy
TPY=${TPY:-0}

function run {
    tpc=${1//.py/.tpc}
    if [[ "${tpc}" == "$1" ]]; then
        echo $1 does not end with .py
        exit 1
    fi
    if [[ "${TPY}" == "0" ]]; then
        echo "./tpc -o ${tpc} $1"
        echo "${TPVM} ${tpc}"
        ./tpc -o ${tpc} $1 || return 1
        "${TPVM}" ${tpc} || return 1
    else
        echo "./tpy $1"
        ./tpy $1 || return 1
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
