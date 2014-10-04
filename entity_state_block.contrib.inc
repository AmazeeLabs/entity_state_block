<?php

/**
 * @file
 * Default implementations of the entity_state_block hooks.
 */

// @todo: more implementations: panelizer state, revisions, last node update by, translations state, etc.

if (!function_exists('node_entity_state_block_info')) {

  /**
   * Implements hook_entity_state_block_info().
   */
  function node_entity_state_block_info() {
    return array(
      'node' => array(
        'node_status' => array(
          'title' => t('Content status'),
          'description' => t('Whether a node is published or not.'),
          'weight' => -100,
          'enabled' => TRUE,
        ),
        'node_author' => array(
          'title' => t('Content author'),
          'description' => t('The value of the "Authored by" content property.'),
        ),
      ),
    );
  }

}
if (!function_exists('node_entity_state_block_view')) {

  /**
   * Implements hook_entity_state_block_view().
   */
  function node_entity_state_block_view($entity_type, $entity, $enabled_sections) {
    $return = array();
    if ($entity_type == 'node') {
      if (in_array('node_status', $enabled_sections)) {
        $return['node_status'] = array(
          'title' => t('Status'),
          'value' => $entity->status == NODE_PUBLISHED ? t('Published') : t('Unpublished'),
        );
      }
      if (in_array('node_author', $enabled_sections)) {
        $return['node_author'] = array(
          'title' => t('Authored by'),
          'value' => theme('username', array('account' => $entity)),
        );
      }
    }
    return $return;
  }
}

if (!function_exists('domain_entity_state_block_info')) {

  /**
   * Implements hook_entity_state_block_info().
   */
  function domain_entity_state_block_info() {
    return array(
      'node' => array(
        'domains' => array(
          'title' => t('Domains'),
          'description' => t('Assigned node domains.'),
        ),
      ),
    );
  }

}
if (!function_exists('domain_entity_state_block_view')) {

  /**
   * Implements hook_entity_state_block_view().
   */
  function domain_entity_state_block_view($entity_type, $entity, $enabled_sections) {
    $return = array();
    if ($entity_type == 'node') {
      if (in_array('domains', $enabled_sections)) {
        $all_affiliates = !empty($entity->domain_site);
        $domain_names = array();
        if (!empty($entity->domains)) {
          $domains = array_intersect_key(domain_domains(), $entity->domains);
          foreach ($domains as $domain) {
            $domain_names[] = '<span style="white-space: nowrap;">' . $domain['sitename'] . '</span>';
          }
        }
        $value = t('All affiliates') . ': ' . ($all_affiliates ? t('yes') : t('no')) . '<br />'
            . t('Published to') . ': ' . implode(', ', $domain_names);
        $return['domains'] = array(
          'title' => t('Domains'),
          'value' => $value,
        );
      }
    }
    return $return;
  }
}
