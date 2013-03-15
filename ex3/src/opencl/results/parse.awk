/N/ {
    printf " N: %d", $2
}

/Local/ {
    printf " Block Size: %d", $3
    }

/time/ {
    printf " time: %f\n", $3
    }
