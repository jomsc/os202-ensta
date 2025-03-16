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

// Fonction pour déterminer si un processus possède le foyer initial
bool has_initial_fire(int rank, int num_procs, unsigned discretization, unsigned start_row) {
    // Calcul du nombre de lignes par processus
    int rows_per_proc = discretization / (num_procs - 1);
    int extra_rows = discretization % (num_procs - 1);

    // Calcul de la plage de lignes pour ce processus
    int start_idx = 0;
    for (int i = 1; i < rank; ++i) {
        start_idx += rows_per_proc + (i <= extra_rows ? 1 : 0);
    }

    int end_idx = start_idx + rows_per_proc + (rank <= extra_rows ? 1 : 0);

    // Vérifier si le foyer initial est dans cette plage
    return (start_row >= start_idx && start_row < end_idx);
}

// Fonction pour créer un modèle partiel adapté à la tranche de domaine
Model create_partial_model(const ParamsType& params, int rank, int size) {
    // Calculer le nombre de lignes par processus
    int num_compute_procs = size - 1; // Exclure le processus 0 (affichage)
    int rows_per_proc = params.discretization / num_compute_procs;
    int extra_rows = params.discretization % num_compute_procs;

    // Déterminer l'indice de début et de fin pour ce processus
    int proc_rank = rank - 1; // Processus 0 est réservé pour l'affichage
    int start_row = 0;
    for (int i = 0; i < proc_rank; ++i) {
        start_row += rows_per_proc + (i < extra_rows ? 1 : 0);
    }
    int local_rows = rows_per_proc + (proc_rank < extra_rows ? 1 : 0);

    // Créer un modèle local pour cette tranche
    Model::LexicoIndices adjusted_start = params.start;
    if (has_initial_fire(rank, size, params.discretization, params.start.row)) {
        // Ajuster la position du foyer initial pour ce processus
        adjusted_start.row -= start_row;
    } else {
        // Pas de foyer initial dans cette tranche
        adjusted_start.row = params.discretization + 1; // Hors limites
    }

    return Model(params.length, params.discretization, params.wind, adjusted_start);
}

int main(int nargs, char* args[])
{
    // Initialisation de MPI
    MPI_Init(&nargs, &args);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Vérifier qu'il y a au moins 2 processus (1 pour l'affichage, au moins 1 pour le calcul)
    if (size < 2) {
        if (rank == 0) {
            std::cerr << "Erreur : au moins 2 processus MPI sont nécessaires" << std::endl;
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // Création du groupe de communication pour les processus de calcul
    MPI_Comm compute_comm;
    MPI_Group world_group, compute_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);

    // Tous les processus sauf le rang 0 font partie du groupe de calcul
    std::vector<int> compute_ranks(size - 1);
    for (int i = 0; i < size - 1; ++i) {
        compute_ranks[i] = i + 1;
    }

    MPI_Group_incl(world_group, size - 1, compute_ranks.data(), &compute_group);
    MPI_Comm_create(MPI_COMM_WORLD, compute_group, &compute_comm);

    // Lecture des paramètres de la ligne de commande
    auto params = parse_arguments(nargs - 1, &args[1]);

    // Diffusion des paramètres à tous les processus
    MPI_Bcast(&params.length, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&params.discretization, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    MPI_Bcast(&params.wind, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&params.start.column, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    MPI_Bcast(&params.start.row, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        display_params(params);
        if (!check_params(params)) {
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            return EXIT_FAILURE;
        }
    }

    // Synchronisation pour s'assurer que tous les processus ont les paramètres corrects
    MPI_Barrier(MPI_COMM_WORLD);

    // Calcul du découpage en tranches
    int num_compute_procs = size - 1;
    int rows_per_proc = params.discretization / num_compute_procs;
    int extra_rows = params.discretization % num_compute_procs;

    // Création des tableaux pour stocker les tailles et déplacements
    std::vector<int> row_counts(num_compute_procs);
    std::vector<int> row_displacements(num_compute_procs);

    int current_disp = 0;
    for (int i = 0; i < num_compute_procs; ++i) {
        row_counts[i] = rows_per_proc + (i < extra_rows ? 1 : 0);
        row_displacements[i] = current_disp;
        current_disp += row_counts[i];
    }

    // Initialisation du modèle pour chaque processus
    Model* simu = nullptr;

    // Variables pour mesurer le temps
    auto start_time = std::chrono::high_resolution_clock::now();
    auto compute_start_time = std::chrono::high_resolution_clock::now();
    auto comm_start_time = std::chrono::high_resolution_clock::now();
    double compute_time = 0.0;
    double comm_time = 0.0;

    if (rank == 0) {
        // Processus d'affichage
        auto displayer = Displayer::init_instance(params.discretization, params.discretization);

        // Buffers pour stocker les données globales
        std::vector<std::uint8_t> vegetation_map(params.discretization * params.discretization);
        std::vector<std::uint8_t> fire_map(params.discretization * params.discretization);

        while (true) {
            // Recevoir les résultats
            for (int i = 1; i < size; ++i) {
                int start = row_displacements[i - 1];
                int end = start + row_counts[i - 1];

                std::vector<std::uint8_t> local_vegetation(row_counts[i - 1] * params.discretization);
                std::vector<std::uint8_t> local_fire(row_counts[i - 1] * params.discretization);
                MPI_Recv(local_vegetation.data(), local_vegetation.size(), MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(local_fire.data(), local_fire.size(), MPI_UNSIGNED_CHAR, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                for (int j = start; j < end; ++j) {
                    std::copy(local_vegetation.begin() + (j - start) * params.discretization,
                              local_vegetation.begin() + (j - start + 1) * params.discretization,
                              vegetation_map.begin() + j * params.discretization);
                    std::copy(local_fire.begin() + (j - start) * params.discretization,
                              local_fire.begin() + (j - start + 1) * params.discretization,
                              fire_map.begin() + j * params.discretization);
                }
            }

            if ((simu->time_step() & 31) == 0)
                std::cout << "Time step " << simu->time_step() << "\n===============" << std::endl;
            displayer->update(vegetation_map, fire_map);

            SDL_Event event;
            if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
                break;
        }
    } else {
        // Initialisation du modèle partiel
        simu = new Model(create_partial_model(params, rank, size));

        while (true) {
            compute_start_time = std::chrono::high_resolution_clock::now();
            if (!simu->update()) break;
            compute_time += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - compute_start_time).count();

            comm_start_time = std::chrono::high_resolution_clock::now();
            int start = row_displacements[rank - 1];
            int end = start + row_counts[rank - 1];
            MPI_Send(simu->vegetal_map().data() + start * params.discretization, (end - start) * params.discretization, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
            MPI_Send(simu->fire_map().data() + start * params.discretization, (end - start) * params.discretization, MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD);
            comm_time += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - comm_start_time).count();

            std::this_thread::sleep_for(0.1s);
        }

        delete simu;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    if (rank == 0) {
        std::cout << "Temps global moyen par itération : " << total_time / double(simu->time_step()) << " ms\n";
    } else {
        std::cout << "Processus " << rank << " - Temps de calcul : " << compute_time << " ms, Temps de communication : " << comm_time << " ms\n";
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
