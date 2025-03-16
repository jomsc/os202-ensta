#include <string>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <thread>
#include <mpi.h>
#include <chrono>

#include "model.hpp"
#include "display.hpp"

using namespace std::string_literals;
using namespace std::chrono_literals;

struct ParamsType
{
    double length{1.};
    unsigned discretization{20u};
    std::array<double,2> wind{0.,0.};
    Model::LexicoIndices start{10u,10u};
};

void analyze_arg( int nargs, char* args[], ParamsType& params )
{
    if (nargs ==0) return;
    std::string key(args[0]);
    if (key == "-l"s)
    {
        if (nargs < 2)
        {
            std::cerr << "Manque une valeur pour la longueur du terrain !" << std::endl;
            exit(EXIT_FAILURE);
        }
        params.length = std::stoul(args[1]);
        analyze_arg(nargs-2, &args[2], params);
        return;
    }
    auto pos = key.find("--longueur=");
    if (pos < key.size())
    {
        auto subkey = std::string(key,pos+11);
        params.length = std::stoul(subkey);
        analyze_arg(nargs-1, &args[1], params);
        return;
    }

    if (key == "-n"s)
    {
        if (nargs < 2)
        {
            std::cerr << "Manque une valeur pour le nombre de cases par direction pour la discrétisation du terrain !" << std::endl;
            exit(EXIT_FAILURE);
        }
        params.discretization = std::stoul(args[1]);
        analyze_arg(nargs-2, &args[2], params);
        return;
    }
    pos = key.find("--number_of_cases=");
    if (pos < key.size())
    {
        auto subkey = std::string(key, pos+18);
        params.discretization = std::stoul(subkey);
        analyze_arg(nargs-1, &args[1], params);
        return;
    }

    if (key == "-w"s)
    {
        if (nargs < 2)
        {
            std::cerr << "Manque une paire de valeurs pour la direction du vent !" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::string values =std::string(args[1]);
        params.wind[0] = std::stod(values);
        auto pos = values.find(",");
        if (pos == values.size())
        {
            std::cerr << "Doit fournir deux valeurs séparées par une virgule pour définir la vitesse" << std::endl;
            exit(EXIT_FAILURE);
        }
        auto second_value = std::string(values, pos+1);
        params.wind[1] = std::stod(second_value);
        analyze_arg(nargs-2, &args[2], params);
        return;
    }
    pos = key.find("--wind=");
    if (pos < key.size())
    {
        auto subkey = std::string(key, pos+7);
        params.wind[0] = std::stoul(subkey);
        auto pos = subkey.find(",");
        if (pos == subkey.size())
        {
            std::cerr << "Doit fournir deux valeurs séparées par une virgule pour définir la vitesse" << std::endl;
            exit(EXIT_FAILURE);
        }
        auto second_value = std::string(subkey, pos+1);
        params.wind[1] = std::stod(second_value);
        analyze_arg(nargs-1, &args[1], params);
        return;
    }

    if (key == "-s"s)
    {
        if (nargs < 2)
        {
            std::cerr << "Manque une paire de valeurs pour la position du foyer initial !" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::string values =std::string(args[1]);
        params.start.column = std::stod(values);
        auto pos = values.find(",");
        if (pos == values.size())
        {
            std::cerr << "Doit fournir deux valeurs séparées par une virgule pour définir la position du foyer initial" << std::endl;
            exit(EXIT_FAILURE);
        }
        auto second_value = std::string(values, pos+1);
        params.start.row = std::stod(second_value);
        analyze_arg(nargs-2, &args[2], params);
        return;
    }
    pos = key.find("--start=");
    if (pos < key.size())
    {
        auto subkey = std::string(key, pos+8);
        params.start.column = std::stoul(subkey);
        auto pos = subkey.find(",");
        if (pos == subkey.size())
        {
            std::cerr << "Doit fournir deux valeurs séparées par une virgule pour définir la vitesse" << std::endl;
            exit(EXIT_FAILURE);
        }
        auto second_value = std::string(subkey, pos+1);
        params.start.row = std::stod(second_value);
        analyze_arg(nargs-1, &args[1], params);
        return;
    }
}

ParamsType parse_arguments( int nargs, char* args[] )
{
    if (nargs == 0) return {};
    if ( (std::string(args[0]) == "--help"s) || (std::string(args[0]) == "-h") )
    {
        std::cout << 
R"RAW(Usage : simulation [option(s)]
  Lance la simulation d'incendie en prenant en compte les [option(s)].
  Les options sont :
    -l, --longueur=LONGUEUR     Définit la taille LONGUEUR (réel en km) du carré représentant la carte de la végétation.
    -n, --number_of_cases=N     Nombre n de cases par direction pour la discrétisation
    -w, --wind=VX,VY            Définit le vecteur vitesse du vent (pas de vent par défaut).
    -s, --start=COL,ROW         Définit les indices I,J de la case où commence l'incendie (milieu de la carte par défaut)
)RAW";
        exit(EXIT_SUCCESS);
    }
    ParamsType params;
    analyze_arg(nargs, args, params);
    return params;
}

bool check_params(ParamsType& params)
{
    bool flag = true;
    if (params.length <= 0)
    {
        std::cerr << "[ERREUR FATALE] La longueur du terrain doit être positive et non nulle !" << std::endl;
        flag = false;
    }

    if (params.discretization <= 0)
    {
        std::cerr << "[ERREUR FATALE] Le nombre de cellules par direction doit être positive et non nulle !" << std::endl;
        flag = false;
    }

    if ( (params.start.row >= params.discretization) || (params.start.column >= params.discretization) )
    {
        std::cerr << "[ERREUR FATALE] Mauvais indices pour la position initiale du foyer" << std::endl;
        flag = false;
    }
    
    return flag;
}

void display_params(ParamsType const& params)
{
    std::cout << "Parametres définis pour la simulation : \n"
              << "\tTaille du terrain : " << params.length << std::endl 
              << "\tNombre de cellules par direction : " << params.discretization << std::endl 
              << "\tVecteur vitesse : [" << params.wind[0] << ", " << params.wind[1] << "]" << std::endl
              << "\tPosition initiale du foyer (col, ligne) : " << params.start.column << ", " << params.start.row << std::endl;
}

int main(int nargs, char* args[])
{
    // Initialisation de MPI
    MPI_Init(&nargs, &args);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    auto params = parse_arguments(nargs - 1, &args[1]);
    if (rank == 0) {
        display_params(params);
        if (!check_params(params)) {
            MPI_Finalize();
            return EXIT_FAILURE;
        }
    }

    auto displayer = rank == 0 ? Displayer::init_instance(params.discretization, params.discretization) : nullptr;
    auto simu = Model(params.length, params.discretization, params.wind, params.start);

    unsigned long iterations = 0;

    SDL_Event event;

    auto start_time = std::chrono::high_resolution_clock::now();
    double affichage_mean = 0;
    double avancement_mean = 0;

    auto avancement_start = std::chrono::high_resolution_clock::now();
    while (iterations < 842)
    {
        if (rank != 0) {
            // Seuls les processus non nuls effectuent les calculs
            if (!simu.update()) break; // Sortir de la boucle si la simulation est terminée
            MPI_Send(simu.vegetal_map().data(), simu.vegetal_map().size(), MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
            MPI_Send(simu.fire_map().data(), simu.fire_map().size(), MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD);
            //if ((simu.time_step() & 31) == 0)
                //std::cout << "Time step " << simu.time_step() << "\n===============" << std::endl;

        } else {
            std::vector<std::uint8_t> vegetation_map(simu.vegetal_map().size());
            std::vector<std::uint8_t> fire_map(simu.fire_map().size());
            for (int i = 1; i < size; ++i) {
                MPI_Recv(vegetation_map.data(), vegetation_map.size(), MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(fire_map.data(), fire_map.size(), MPI_UNSIGNED_CHAR, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            
            auto avancement_end = std::chrono::high_resolution_clock::now();
            avancement_mean += std::chrono::duration_cast<std::chrono::milliseconds>(avancement_end - avancement_start).count();

            auto affichage_start = std::chrono::high_resolution_clock::now();
            displayer->update(vegetation_map, fire_map);
            auto affichage_end = std::chrono::high_resolution_clock::now();  
            affichage_mean += std::chrono::duration_cast<std::chrono::milliseconds>(affichage_end - affichage_start).count();
        
            if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
                break;
            
            avancement_start = std::chrono::high_resolution_clock::now();
        }

        iterations++;

        //std::this_thread::sleep_for(0.1s);
    }
    
    if (rank==0) {
        auto end_time = std::chrono::high_resolution_clock::now();
        double total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        std::cout << "Temps global moyen par itération : " << total_time / iterations << " ms" << std::endl;
        std::cout << "Temps d'avancement moyen par itération : " << avancement_mean / iterations << " ms" << std::endl;
        std::cout << "Temps d'affichage moyen par itération : " << affichage_mean / iterations << " ms" << std::endl;
        std::cout << "Nombre d'itérations : " << iterations << std::endl;
    }
    
    MPI_Finalize();
    return EXIT_SUCCESS;
}