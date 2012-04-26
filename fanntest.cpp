#include "fann.h"
int main()
{
    const unsigned int num_layers = 4;

    const unsigned int num_input = 8;
    const unsigned int num_output = 8;

    const unsigned int num_neurons_hidden_a = 3;
    const unsigned int num_neurons_hidden_b = 4;

    const float desired_error = (const float) 0.001;
    const unsigned int max_epochs = 500000;
    const unsigned int epochs_between_reports = 1000;
    struct fann *ann = fann_create_standard(
                                            num_layers, 
                                            num_input,
                                            num_neurons_hidden_a, 
                                            num_neurons_hidden_b, 
                                            num_output);
    fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
    fann_train_on_file(ann, "identity.data", max_epochs,
        epochs_between_reports, desired_error);
    fann_save(ann, "identity_float.net");
    fann_destroy(ann);
    return 0;
}
