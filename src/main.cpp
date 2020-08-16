#include "ldp.h"
#include "schema.h"

int main(int argc, char* argv[])
{
    // return main_ldp(argc, argv);
    ldp_schema schema;
    ldp_schema::make_default_schema(&schema);
    printf("var defaultFolioSchema = folioSchema{\n");
    printf("\t[]folioTableSchema{\n");
    for (auto& table : schema.tables) {
        printf("\t\tfolioTableSchema{\n");
        printf("\t\t\tname: \"%s\",\n", table.name.data());
        printf("\t\t\tmoduleName: \"%s\",\n", table.module_name.data());
        printf("\t\t\tsourceSpec: \"%s\",\n", table.source_spec.data());
        printf("\t\t},\n");
    }
    printf("\t},\n");
    printf("}\n");
}

