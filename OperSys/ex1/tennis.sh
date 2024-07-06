#!/bin/bash
#Roy Ronen 215229865

state=0
lines=" --------------------------------- "
ord=" |       |       #       |       | "
opt3=" |       |       #       |       |O"
opt2=" |       |       #       |   O   | "
opt1=" |       |       #   O   |       | "
opt0=" |       |       O       |       | "
opt1o=" |       |   O   #       |       | "
opt2o=" |   O   |       #       |       | "
opt3o="O|       |       #       |       | "
p1po=50
p2po=50

printBoard() {
    echo " Player 1: ${p1po}         Player 2: ${p2po} "
    echo "$lines"
    echo "$ord"
    echo "$ord"
    echo "$1"
    echo "$ord"
    echo "$ord"
    echo "$lines"
}

ballPosition() {
    local state="$3"
    if [[ $1 -lt $2 ]]; then
        if [ "$state" -lt 0 ]; then
            state=0
        fi
        state=$((state+1))
    fi
    if [[ $1 -gt $2 ]]; then
        if [ "$state" -gt 0 ]; then
            state=0
        fi
        state=$((state-1))
    fi
    echo "$state"
}

checkWin() {
    case $3 in
        -3)
            echo "PLAYER 1 WINS !"
            return 1
            ;;
        3)
            echo "PLAYER 2 WINS !"
            return 1
            ;;
    esac
    if [ "$1" -eq 0 ]; then
        if [ "$2" -gt 0 ]; then
            echo "PLAYER 2 WINS !"
            return 1
        else
            if [ "$3" -eq "0" ]; then
                echo "IT'S A DRAW !"
                return 1
            fi
            if [ "$3" -lt "0" ]; then
                echo "PLAYER 1 WINS !"
                return 1
            else
                echo "PLAYER 2 WINS !"
                return 1
            fi
        fi
    fi
    if [ "$2" -eq 0 ]; then
        if [ "$1" -gt 0 ]; then
            echo "PLAYER 1 WINS !"
            return 1
        fi
    fi
    return 0
}

printBoard "$opt0"
while true; do
    echo "PLAYER 1 PICK A NUMBER: "
    read -s p1choice
    reg='^[0-9]+$'
    while ! [[ $p1choice =~ $reg ]] || [ "$p1choice" -gt $p1po ]; do
        echo "NOT A VALID MOVE !"
        echo "PLAYER 1 PICK A NUMBER: "
        read -s p1choice
    done
    echo "PLAYER 2 PICK A NUMBER: "
    read -s p2choice
    reg='^[0-9]+$'
    while ! [[ $p2choice =~ $reg ]] || [ "$p2choice" -gt $p1po ]; do
        echo "NOT A VALID MOVE !"
        echo "PLAYER 2 PICK A NUMBER: "
        read -s p2choice
    done

    state="$(ballPosition "$p1choice" "$p2choice" "$state")"

    p1po=$((p1po-p1choice))
    p2po=$((p2po-p2choice))

    case $state in
        -3)
            printBoard "$opt3"
            ;;
        
        -2)
            printBoard "$opt2"
            ;;

        -1)
            printBoard "$opt1"
            ;;
        0)
            printBoard "$opt0"
            ;;
        1)
            printBoard "$opt1o"
            ;;
        2)
            printBoard "$opt2o"
            ;;
        3)
            printBoard "$opt3o"
            ;;
    esac
    echo -e "       Player 1 played: ${p1choice}\n       Player 2 played: ${p2choice}\n\n"
    checkWin "$p1po" "$p2po" "$state"
    if [ $? -eq 1 ]; then
        break
    fi
done