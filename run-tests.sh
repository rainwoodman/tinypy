TESTS=$@

function run {
    echo "./tpy $1"
    ./tpy $1
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
