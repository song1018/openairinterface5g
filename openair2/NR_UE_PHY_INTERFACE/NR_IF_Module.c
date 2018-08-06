/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/* \file NR_IF_Module.c
 * \brief functions for NR UE FAPI-like interface
 * \author R. Knopp, K.H. HSU
 * \date 2018
 * \version 0.1
 * \company Eurecom / NTUST
 * \email: knopp@eurecom.fr, kai-hsiang.hsu@eurecom.fr
 * \note
 * \warning
 */

#include "NR_IF_Module.h"
#include "mac_proto.h"

#include <stdio.h>

#define MAX_IF_MODULES 100

static nr_ue_if_module_t *nr_ue_if_module_inst[MAX_IF_MODULES];


int8_t handle_bcch_bch(uint8_t *pduP, uint8_t additional_bits, uint32_t ssb_index, uint32_t l_ssb){

    //  pdu_len = 4, 32bits
    //uint8_t extra_bits = pduP[0];
    nr_ue_decode_mib(   (module_id_t)0,
                        0,
                        0,
                        additional_bits,
                        l_ssb,  //  Lssb = 64 is not support    
                        ssb_index,
                        &pduP[1] );



    return 0;
}

int8_t handle_bcch_dlsch(uint32_t pdu_len, uint8_t *pduP){

    return 0;
}

int8_t nr_ue_ul_indication(nr_uplink_indication_t *ul_info){

    NR_UE_L2_STATE_t ret;
    module_id_t module_id = ul_info->module_id;
    NR_UE_MAC_INST_t *mac = get_mac_inst(module_id);

    ret = nr_ue_scheduler(
        ul_info->module_id,
        ul_info->gNB_index,
        ul_info->cc_id,
        ul_info->frame,
        ul_info->slot,
        ul_info->ssb_index, 
        0, 0); //  TODO check tx/rx frame/slot is need for NR version

    switch(ret){
        case CONNECTION_OK:
            break;
        case CONNECTION_LOST:
            break;
        case PHY_RESYNCH:
            break;
        case PHY_HO_PRACH:
            break;
        default:
            break;
    }


    mac->if_module->scheduled_response(&mac->scheduled_response);

    return 0;
}

int8_t nr_ue_dl_indication(nr_downlink_indication_t *dl_info){
    
    int32_t i;
    module_id_t module_id = dl_info->module_id;
    NR_UE_MAC_INST_t *mac = get_mac_inst(module_id);

    //  clean up scheduled_response structure

    if(dl_info->rx_ind != NULL){
        printf("[L2][IF MODULE][DL INDICATION][RX_IND]\n");
        for(i=0; i<dl_info->rx_ind->number_pdus; ++i){
            switch(dl_info->rx_ind->rx_request_body[i].pdu_type){
                case FAPI_NR_RX_PDU_TYPE_MIB:
                        handle_bcch_bch(dl_info->rx_ind->rx_request_body[i].mib_pdu.pdu, dl_info->rx_ind->rx_request_body[i].mib_pdu.additional_bits, dl_info->rx_ind->rx_request_body[i].mib_pdu.ssb_index, dl_info->rx_ind->rx_request_body[i].mib_pdu.l_ssb);
                    break;
                case FAPI_NR_RX_PDU_TYPE_SIB:
                        
                    break;
                case FAPI_NR_RX_PDU_TYPE_DLSCH:
                    break;
                default:
                    break;

            }
        }
        
    }

    if(dl_info->dci_ind != NULL){

    }

    if(nr_ue_if_module_inst[module_id] != NULL){
        nr_ue_if_module_inst[module_id]->scheduled_response(&mac->scheduled_response);
    }


    return 0;
}

nr_ue_if_module_t *nr_ue_if_module_init(uint32_t module_id){

    if (nr_ue_if_module_inst[module_id] == NULL) {
        nr_ue_if_module_inst[module_id] = (nr_ue_if_module_t *)malloc(sizeof(nr_ue_if_module_t));
        memset((void*)nr_ue_if_module_inst[module_id],0,sizeof(nr_ue_if_module_t));

        nr_ue_if_module_inst[module_id]->cc_mask=0;
        nr_ue_if_module_inst[module_id]->current_frame = 0;
        nr_ue_if_module_inst[module_id]->current_slot = 0;
        nr_ue_if_module_inst[module_id]->phy_config_request = NULL;
        nr_ue_if_module_inst[module_id]->scheduled_response = NULL;
        nr_ue_if_module_inst[module_id]->dl_indication = nr_ue_dl_indication;
        nr_ue_if_module_inst[module_id]->ul_indication = nr_ue_ul_indication;
    }

    return nr_ue_if_module_inst[module_id];
}

int8_t nr_ue_if_module_kill(uint32_t module_id) {

    if (nr_ue_if_module_inst[module_id] != NULL){
        free(nr_ue_if_module_inst[module_id]);
    } 
    return 0;
}