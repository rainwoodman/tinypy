TPVM=./tpvm
if [[ "$1" == "-dbg" ]]; then
    TPVM=./tpvm-dbg
    shift
fi

TESTS=$@

function run {
    tpc=${1//.py/.tpc}
    if [[ "${tpc}" == "$1" ]]; then
        echo $1 does not end with .py
        exit 1
    fi
    echo "./tpc -o ${tpc} $1"
    echo "${TPVM} ${tpc}"
    ./tpc -o ${tpc} $1 || return 255
    "${TPVM}" ${tpc}
}

st=0
for i in ${TESTS[@]}; do
    if ! run "${i}"; then
        st=1
    fi
done

if [ "${st}" -ne 0 ]; then
    echo Some tests failed.
    exit 255
else
    echo All tests passed.
fi
