#include "proto_ident.h"
#include "proto_ident_truman.h"
#include "proto_ident_opendpi.h"
#include "helper_file.h"
#include "helper_net.h"

#include <common/msg.h>
#include <common/configuration.h>

#include <stdlib.h>

ProtoIdent* pi_create(const Configuration& config)
{
	/* TODO: We only have the builtin identification mechanism. Improve this!!! */
	return new ProtoIdentTruman(config);
}

