/*PGR-GNU*****************************************************************

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Copyright (c) 2018 Sourabh Garg
Mail to: sourabh.garg.mat14@gmail.com

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

********************************************************************PGR-GNU*/

#ifndef INCLUDE_BELLMAN_FORD_PGR_BELLMAN_FORD_HPP_
#define INCLUDE_BELLMAN_FORD_PGR_BELLMAN_FORD_HPP_
#pragma once

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <deque>
#include <set>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>
#include <functional>
#include <limits>
#include "cpp_common/pgr_messages.h"
#include "cpp_common/basePath_SSEC.hpp"
#include "cpp_common/pgr_base_graph.hpp"




//******************************************

template < class G >
class Pgr_bellman_ford : public pgrouting::Pgr_messages {
 public:
     typedef typename G::V V;
     typedef typename G::E E;
     typename G::EO_i out, out_end;
     //@}

     //! @name BellmanFord
     //@{
     //! BellmanFord 1 to 1
     Path bellman_ford(
             G &graph,
             int64_t start_vertex,
             int64_t end_vertex,
             bool only_cost = false) {
         clear();
         log << std::string(__FUNCTION__) << "\n";
        
         // adjust predecessors and distances vectors
         predecessors.resize(graph.num_vertices());
         distances.resize(graph.num_vertices());


         if (!graph.has_vertex(start_vertex)
                 || !graph.has_vertex(end_vertex)) {
             return Path(start_vertex, end_vertex);
         }

         // get the graphs source and target
         auto v_source(graph.get_V(start_vertex));
         auto v_target(graph.get_V(end_vertex));

         // perform the algorithm
         bellman_ford_1_to_1(graph, v_source, v_target);

         // get the results
         return Path(
                 graph,
                 v_source, v_target,
                 predecessors, distances,
                 only_cost, true);
     }


     //! BellmanFord 1 to many
     std::deque<Path> bellman_ford(
             G &graph,
             int64_t start_vertex,
             const std::vector< int64_t > &end_vertex,
             bool only_cost= false) {
         // adjust predecessors and distances vectors
         clear();
         log << std::string(__FUNCTION__) << "\n";
         predecessors.resize(graph.num_vertices());
         distances.resize(graph.num_vertices());
         log << " in one_to_many"<<"\n";
         // get the graphs source and target
         if (!graph.has_vertex(start_vertex))
             return std::deque<Path>();
         auto v_source(graph.get_V(start_vertex));

         std::set< V > s_v_targets;
         for (const auto &vertex : end_vertex) {
             if (graph.has_vertex(vertex)) {
                 s_v_targets.insert(graph.get_V(vertex));
             }
         }
         
         std::vector< V > v_targets(s_v_targets.begin(), s_v_targets.end());
         // perform the algorithm
         bellman_ford_1_to_many(graph, v_source, v_targets);

         std::deque< Path > paths;
         // get the results // route id are the targets
         paths = get_paths(graph, v_source, v_targets, only_cost);

         std::stable_sort(paths.begin(), paths.end(),
                 [](const Path &e1, const Path &e2)->bool {
                 return e1.end_id() < e2.end_id();
                 });

         return paths;
     }

     // BellmanFord many to 1
     std::deque<Path> bellman_ford(
             G &graph,
             const std::vector < int64_t > &start_vertex,
             int64_t end_vertex,
             bool only_cost = false) {
         std::deque<Path> paths;
         log << std::string(__FUNCTION__) << "\n";
         for (const auto &start : start_vertex) {
             paths.push_back(
                     bellman_ford(graph, start, end_vertex, only_cost));
         }

         std::stable_sort(paths.begin(), paths.end(),
                 [](const Path &e1, const Path &e2)->bool {
                 return e1.start_id() < e2.start_id();
                 });
         return paths;
     }


     // BellmanFord many to many
     std::deque<Path> bellman_ford(
             G &graph,
             const std::vector< int64_t > &start_vertex,
             const std::vector< int64_t > &end_vertex,
             bool only_cost = false) {
         // a call to 1 to many is faster for each of the sources
         std::deque<Path> paths;
         log << std::string(__FUNCTION__) << "\n";
         log << "----------------------------------------------------\n";
         for (auto vi = vertices(graph.graph).first;
                 vi != vertices(graph.graph).second; ++vi) {
             if ((*vi) >= graph.num_vertices()) break;
             log << (*vi) << ": " << " out_edges_of(" << graph.graph[(*vi)] << "):";
             for (boost::tie(out, out_end) = out_edges(*vi, graph.graph);
                     out != out_end; ++out) {
                 log << ' '
                     << graph.graph[*out].id << "=("
                     << graph[graph.source(*out)].id << ", "
                     << graph[graph.target(*out)].id << ") = "
                     << graph.graph[*out].cost <<"\t";
             }
             log << std::endl;
         }
         log <<"-------------------------------------------------\n";
         
         for (const auto &start : start_vertex) {
             auto r_paths = bellman_ford(graph, start, end_vertex, only_cost);
             paths.insert(paths.begin(), r_paths.begin(), r_paths.end());
         }
         
         
         std::sort(paths.begin(), paths.end(),
                 [](const Path &e1, const Path &e2)->bool {
                 return e1.end_id() < e2.end_id();
                 });
         std::stable_sort(paths.begin(), paths.end(),
                 [](const Path &e1, const Path &e2)->bool {
                 return e1.start_id() < e2.start_id();
                 });
         return paths;
     }

     //@}

 private:
     //! Call to BellmanFord  1 source to 1 target
     bool bellman_ford_1_to_1(
                 G &graph,
                 V source,
                 V target) {
         log << std::string(__FUNCTION__) << "\n";
         
         try {
            
             bool negative = boost::bellman_ford_shortest_paths(graph.graph, int(graph.num_vertices()),
                     boost::predecessor_map(&predecessors[0])
                     .weight_map(get(&G::G_T_E::cost, graph.graph))
                     .distance_map(&distances[0])
                     .root_vertex(source)
                     .visitor(MyBellmanVisitor())
                     );
             for(int i=0; i<int(graph.num_vertices()); i++)
             {
                log <<distances[i]<<" : "<<predecessors[i]<<"\n";
             }
             
         } catch (boost::exception const& ex) {
             (void)ex;
             throw;
         } catch (std::exception &e) {
             (void)e;
             throw;
         } catch (...) {
             throw;
         }
         return true;
     }
     //! Call to BellmanFord  1 source to many targets
     bool bellman_ford_1_to_many(
             G &graph,
             V source,
             const std::vector< V > &targets) {
        log << std::string(__FUNCTION__) << "\n";
         try {
            for(int i=0; i<int(graph.num_vertices()); i++)
             {
                if(i == int(graph[source].id)-1)
                    distances[i] = 0;
                else
                    distances[i] = 1000000000;
             }
            bool neg = boost::bellman_ford_shortest_paths(graph.graph, int(graph.num_vertices()),
                     boost::predecessor_map(&predecessors[0])
                     .weight_map(get(&G::G_T_E::cost, graph.graph))
                     .distance_map(&distances[0])
                    // .root_vertex(source)
                     .visitor(MyBellmanVisitor())
                     );
          if(neg == true)
             for(int i=0; i<int(graph.num_vertices()); i++)
             {
                log<< "no cycle\n";
                log <<distances[i]<<" : "<<predecessors[i]<<"\n";
             }
            else
                for(int i=0; i<int(graph.num_vertices()); i++)
             {
                log << "neative cycle\n";
                log <<distances[i]<<" : "<<predecessors[i]<<"\n";
             }
             
         } catch (boost::exception const& ex) {
             (void)ex;
             throw;
         } catch (std::exception &e) {
             (void)e;
             throw;
         } catch (...) {
             throw;
         }
         return true;
     }


     // To Empty predecessors and distances vector for each function call 
     void clear() {
         predecessors.clear();
         distances.clear();
     }



     // used when multiple goals
     std::deque<Path> get_paths(
             const G &graph,
             V source,
             std::vector< V > &targets,
             bool only_cost) const {
        log << std::string(__FUNCTION__) << "\n";
         std::deque<Path> paths;
         for (const auto target : targets) {
             paths.push_back(Path(
                         graph,
                         source, target,
                         predecessors, distances,
                         only_cost, true));
         }
         return paths;
     }



     //! @name members
     //@{
     std::vector< V > predecessors;
     std::vector< double > distances;
     
     //@}



     //! @name Stopping classes
     //@{
     class MyBellmanVisitor : public boost::bellman_visitor<>, public pgrouting::Pgr_messages
        {
        public:
            template <class B_G>
            void examine_edge(E e, B_G& g) const
            {
                log << std::string(__FUNCTION__) << "\n";
                log << "Edge examined: " << source(e, g) << " : " << target(e, g) << " \n ";
            }
            template <class B_G>
            void edge_relaxed(E e, B_G &g)  const
            {
                log << std::string(__FUNCTION__) << "\n";
                log << "Edge relaxed:  " << source(e, g) << " : " << target(e, g) << " \n ";
            }
            template <class B_G>
            void edge_not_relaxed(E e, B_G &g)  const
            {
                log << std::string(__FUNCTION__) << "\n";
                log << "Edge not relaxed:  " << source(e, g) << " : " << target(e, g) << " \n ";
            }
            template <class B_G>
            void edge_minimized(E e, B_G &g)  const
            {
                log << std::string(__FUNCTION__) << "\n";
                log << "-->>Edge minimized:  " << source(e, g) << " : " << target(e, g) << " <<--\n ";
            }
            template <class B_G>
            void edge_not_minimized(E e, B_G &g) const
            {
                log << std::string(__FUNCTION__) << "\n";
                log << "-->> Edge NOT minimized:  " << source(e, g) << " : " << target(e, g) << " <<-- \n ";
            }
        };



     //@}
};


 //namespace
#endif  // INCLUDE_BELLMAN_FORD_PGR_BELLMAN_FORD_HPP_
