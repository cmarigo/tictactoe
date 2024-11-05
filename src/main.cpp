#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <chrono>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() : board{{{' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '}}}, 
                  current_player('X'), game_over(false), winner(' ') {}

    void display_board() {
        // Exibir o tabuleiro no console
        std::lock_guard<std::mutex> lock(board_mutex);
        for (const auto& row : board) {
            for (char cell : row) {
                std::cout << (cell == ' ' ? '-' : cell) << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    bool make_move(char player, int row, int col) {
        // Implementar a lógica para realizar uma jogada no tabuleiro
        // Utilizar mutex para controle de acesso
        // Utilizar variável de condição para alternância de turnos
        std::unique_lock<std::mutex> lock(board_mutex);
       if (board[row][col] == ' ' && !game_over) { // Verifica se a célula está livre
            board[row][col] = player;
            if (check_win(player)) {
                game_over = true;
                winner = player;
            } else if (check_draw()) {
                game_over = true;
                winner = 'D'; // Empate
            }
            current_player = (current_player == 'X') ? 'O' : 'X';
            turn_cv.notify_all();
            return true;
        }
        return false;
    }

    bool check_win(char player) {
        // Verificar se o jogador atual venceu o jogo
         for (int i = 0; i < 3; ++i) {
        if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) || 
            (board[0][i] == player && board[1][i] == player && board[2][i] == player))
            return true;
    }
    if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
        (board[0][2] == player && board[1][1] == player && board[2][0] == player))
        return true;
    
    return false;
    }

    bool check_draw() {
        // Verificar se houve um empate
        for (const auto& row : board){
            for (const auto& cell : row){
                if (cell == ' ') return false;
            }
        }     
        return true;
    }

    bool is_game_over() {
        // Retornar se o jogo terminou
        return game_over;  
    }

    char get_winner() {
        // Retornar o vencedor do jogo ('X', 'O', ou 'D' para empate)
        return winner;
    }

     char get_current_player() {
        return current_player;
    }


    void wait_for_turn(char player) {
        std::unique_lock<std::mutex> lock(board_mutex);
        turn_cv.wait(lock, [&] { return current_player == player || game_over; });
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe& g, char s, std::string strat) 
        : game(g), symbol(s), strategy(strat) {}

    void play() {
        // Executar jogadas de acordo com a estratégia escolhida
       while (!game.is_game_over()) {
            game.wait_for_turn(symbol);
            if (game.is_game_over()) break;
            game.display_board();
            if (strategy == "sequential") {
                play_sequential();
            } else if (strategy == "random") {
                play_random();
            }
        }
    }

private:
    void play_sequential() {
        // Implementar a estratégia sequencial de jogadas
        for (int i = 0; i < 3 ; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (game.make_move(symbol, i, j)) return;
            }
        }
    }

    void play_random() {
        // Implementar a estratégia aleatória de jogadas
        while (true) {
            int row = rand() % 3;
            int col = rand() % 3;
            if (game.make_move(symbol, row, col)) return;
    }
    }
};

// Função principal
int main() {
     std::srand(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count())); // Inicializa o gerador de números aleatórios

    // Inicializar o jogo e os jogadores
    TicTacToe game;
    Player player1(game, 'X', "sequential");
    Player player2(game, 'O', "random");

    // Criar as threads para os jogadores
    std::thread t1(&Player::play, &player1);
    std::thread t2(&Player::play, &player2);

    // Aguardar o término das threads
    t1.join();
    t2.join();

    // Exibir o resultado final do jogo
    game.display_board();
    if (game.get_winner() == 'D')
        std::cout << "O jogo terminou em empate!\n";
    else
        std::cout << "O vencedor é: " << game.get_winner() << "\n";

    return 0;
}
