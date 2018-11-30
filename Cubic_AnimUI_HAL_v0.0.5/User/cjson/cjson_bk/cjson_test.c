#include "cJSON.h"

//jansson Test
void jansson_pack_test(void)
{
    json_t *root;
    char *out;

    /* Build an empty JSON object */
    root = json_pack("{}");

    out = json_dumps(root, JSON_ENCODE_ANY);
    printf("out:%s\r\n", out);
    free(root);
    free(out);

    /* Build the JSON object {"foo": 42, "bar": 7} */
    root = json_pack("{sisi}", "foo", 42, "bar", 7);

    out = json_dumps(root, JSON_ENCODE_ANY);
    printf("out:%s\r\n", out);
    free(root);
    free(out);

    /* Like above, ':', ',' and whitespace are ignored */
    root = json_pack("{s:i, s:i}", "foo", 42, "bar", 7);

    out = json_dumps(root, JSON_ENCODE_ANY);
    printf("out:%s\r\n", out);
    free(root);
    free(out);

    /* Build the JSON array [[1, 2], {"cool": true}] */
    root = json_pack("[[i,i],{s:b}]", 1, 2, "cool", 1);

    out = json_dumps(root, JSON_ENCODE_ANY);
    printf("out:%s\r\n", out);
    free(root);
    free(out);

    /* Build a string from a non-null terminated buffer */
    char buffer[4] = {'t', 'e', 's', 't'};
    root = json_pack("[s#]", buffer, 4);

    out = json_dumps(root, JSON_ENCODE_ANY);
    printf("out:%s\r\n", out);
    free(root);
    free(out);

    /* Concatenate strings together to build the JSON string "foobarbaz" */
    root = json_pack("[s++]", "foo", "bar", "baz");

    out = json_dumps(root, JSON_ENCODE_ANY);
    printf("out:%s\r\n", out);
    free(root);
    free(out);
}





